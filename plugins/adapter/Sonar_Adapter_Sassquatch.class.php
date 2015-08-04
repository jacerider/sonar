<?php

class Sonar_Adapter_Sassquatch extends Sonar_Adapter_Abstract {

  /**
   * Contains @import records so files are not imported more than once.
   */
  protected $includedFiles;


  /**
   * Prepare each SCSS file for aggregation.
   */
  protected function filesPrepareFile($file){
    return $this->filesPrepareFileContents($file['data']);
  }


  /**
   * Prepare each SCSS file for aggregation.
   */
  protected function filesPrepareFileContents($filepath){
    // Get the file contents
    $data = file_get_contents($filepath);
    $this->includedFiles[$filepath] = $filepath;

    // Look for internal imports
    preg_match_all('/(?<!\/\/)(?<!\t)(?<! )@import ["|\'](.*)["|\'];/', $data, $results);

    if(!empty($results[1])){
      $info = pathinfo($filepath);
      foreach($results[1] as $name){
        // Imports that are bourbon are allowed to pass through as SASSquatch
        // support them.
        if(strpos($name, 'bourbon') !== false) continue;

        $importpath = $info['dirname'] . '/' . $name .'.scss';
        // Add an unscore per SCSS to the filename.
        $importpath = str_replace('/' . basename($importpath), '/_' . basename($importpath), $importpath);
        $filedata = '// SONAR IGNORE IMPORT '.$name;
        $pattern = '/(?<!\/\/)(?<!\t)(?<! )@import ["|\']('. str_replace('/', '\/', $name) .')["|\'];/';
        // Files only need to be included once
        if(!isset($this->includedFiles[$importpath])){
          if(file_exists($importpath)){
            $filedata = $this->filesPrepareFileContents($importpath);
          }
        }

        $data = preg_replace($pattern, $filedata, $data);
            dsm($data);
      }
    }

    $data = "// SONAR IMPORT $filepath\n" . $data;

    // TODO: Remove once node-sass has imagePath support
    $data = preg_replace('!image-url\([\'|"](.*?)[\'|"]\)!', 'url('.$this->imagesPath().'/$1)', $data);

    // TODO: Remove once node-sass has fontPath support
    $data = preg_replace('!font-url\([\'|"](.*?)[\'|"]\)!', 'url('.$this->fontsPath().'/$1)', $data);

    return $data;
  }


  /**
   * Get the path to images.
   */
  protected function imagesPath(){
    return !empty($this->settings['assets']['images_path']) ? base_path() . $this->settings['assets']['images_path'] : base_path() . drupal_get_path('theme', $this->theme) . '/assets/images';
  }


  /**
   * Get the path to images.
   */
  protected function fontsPath(){
    return !empty($this->settings['assets']['fonts_path']) ? base_path() . $this->settings['assets']['fonts_path'] : base_path() . drupal_get_path('theme', $this->theme) . '/assets/fonts';
  }


  /**
   * Compile the SASS files.
 *
 * @return
 *   The compiled CSS string.
   */
  protected function compile(){

    // If remote service is down we will only check if it is up every so often.
    if ($cache = cache_get('sonar_remote_fail')) {
      if($cache->expire > time()){
        throw new Exception( 'Sonar service could not be reached. Will try again in 1 minute.');
      }
      cache_clear_all('sonar_remote_fail', 'cache');
    }

    // output_style -> 'nested', 'expanded', 'compact', 'compressed'  -> defaults to compressed

    $scheme =  'http';
    $host = 'api.sassquat.ch';
    $user = $this->settings['api_key'];
    $pass = '';
    $url = $scheme . '://' . $host . '/squeeze';

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_POST, true);

    $data = array(
      'data' => $this->data,
      'output_style' => $this->settings['output_style'],
      'image_path' => $this->imagesPath(),
    );

    curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
    $response = curl_exec($ch);
    $json = json_decode($response);
    $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);

    if($code == 200 && !empty($json) && !empty($json->css)){
      return $json->css;
    }
    else{
      $this->compileError($code, $json, $response);
    }
  }


  /**
   * Handle errors from SASSquatch
   */
  protected function compileError($code, $json, $response){
    switch($code){
      case -1:
        watchdog('sonar', 'Error - Request to Send-the-Style timed out.', NULL, WATCHDOG_DEBUG, NULL);
        cache_set('sonar_remote_fail', 1, 'cache', strtotime('+1 minute'));
        break;
      case 200:
        if(empty($json)){
          watchdog('sonar', 'No data was returned from the remote host.', NULL, WATCHDOG_DEBUG, NULL);
        }
        else{
          if(!empty($json->message)){
            watchdog('sonar', '<pre>' . $json->message . '</pre>', NULL, WATCHDOG_DEBUG, NULL);
          }
          else{
            watchdog('sonar', 'An undefined error occurred.', NULL, WATCHDOG_DEBUG, NULL);
          }
        }
        break;
      case 400:
        watchdog('sonar', 'Bad Request - '.$json->message.'.', NULL, WATCHDOG_DEBUG, NULL);
        break;
      case 401:
        watchdog('sonar', 'Unauthorized - No valid API key provided.', NULL, WATCHDOG_DEBUG, NULL);
        break;
      case 402:
        watchdog('sonar', 'Request Failed - Parameters were valid but request failed.', NULL, WATCHDOG_DEBUG, NULL);
        break;
      case 404:
        watchdog('sonar', 'Not Found - The requested item doesn\'t exist.', NULL, WATCHDOG_DEBUG, NULL);
        break;
      default:
        watchdog('sonar', "Server errors - something went wrong on Send-the-Style\'s end. Response:\n<pre>" . print_r($response, TRUE) . "</pre>", NULL, WATCHDOG_DEBUG, NULL);
        break;
    }

    throw new Exception( t('Could not remote compile the file <code>!file</code>. Please consult your !watchdog for a detailed error description.', array('!file' => basename($this->filepath), '!watchdog' => l('log messages', 'admin/reports/dblog'))) );
  }


  /**
   * An array containing default values for the settings form fields.
   */
  public function settingsDefaults(){
    return array(
      'api_key' => '',
      'output_style' => 'compressed',
      'assets' => array(
        'images_path' => '',
        'fonts_path' => '',
      ),
    );
  }

  /**
   * Configuration settings made available on the Sonar settings page.
   *
   * @param $form
   *   An empty array.
   * @param $form_state
   *   The form state of the parent form.
   */
  public function settingsForm(&$form, &$form_state){

    $form['logo'] = array(
      '#markup' => theme('image', array('path' => drupal_get_path('module','sonar') . '/images/logo-sassquatch.png')),
    );

    $form['api_key'] = array(
      '#title' => t('SASSquatch API Key'),
      '#type' => 'textfield',
      '#default_value' => $this->settings['api_key'],
      '#description' => t('Get your API key by visiting !url.', array('!url' => l('SASSquat.ch', 'http://sassquat.ch'))),
    );

    $form['output_style'] = array(
      '#title' => t('Output Style'),
      '#type' => 'select',
      '#options' => array('nested' => t('Nested'), 'expanded' => t('Expanded'), 'compact' => t('Compact'), 'compressed' => t('Compressed')),
      '#default_value' => $this->settings['output_style'],
      '#description' => t('The style of the returned CSS.'),
      '#required' => TRUE,
    );

    $form['assets'] = array(
      '#type' => 'fieldset',
      '#title' => t('Assets location'),
    );
    $form['assets']['description'] = array(
      '#markup' => '<div class="description">'. t('Tell us where your assets are located on your server. When writing your SASS make sure to use image-url("EXAMPLE.png") when referencing images and font-url("EXAMPLE.ttf") when referencing fonts. Defaults to <code>!files</code>.', array('!files' => '[CURRENT THEME]/assets/[type]')) . '</div>',
    );
    $form['assets']['images_path'] = array(
      '#title' => t('Images'),
      '#type' => 'textfield',
      '#attributes' => array(
        'placeholder' => t('e.g. /path/to/images or http://yourhost.com/path/to/images'),
      ),
      '#default_value' => $this->settings['assets']['images_path'],
    );
    $form['assets']['fonts_path'] = array(
      '#title' => t('Fonts'),
      '#type' => 'textfield',
      '#attributes' => array(
        'placeholder' => t('e.g. /path/to/fonts or http://yourhost.com/path/to/fonts'),
      ),
      '#default_value' => $this->settings['assets']['fonts_path'],
    );

  }

}
