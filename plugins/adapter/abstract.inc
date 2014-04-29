<?php

/**
 * @file
 * Abstraction of the selection logic of a sonar adapter.
 *
 * Implementations that wish to provide an implementation of this should
 * register it using CTools' plugin system.
 */
interface Sonar_Adapter {

  /**
   * Constructor for the links.
   *
   * @param $type
   *   The name of the links plugin.
   */
  public function __construct($type, $args = NULL);

}

/**
 * An abstract implementation of Sonar_Adapter.
 */
abstract class Sonar_Adapter_Abstract implements Sonar_Adapter {

  /**
   * The name of the links plugin.
   */
  protected $type;

  /**
   * The plugin definition.
   */
  protected $plugin;

  /**
   * An array of all SCSS items being requested on the page.
   */
  protected $scss;

  /**
   * The ID of the SCSS collection.
   */
  protected $id;

  /**
   * The current theme key.
   */
  protected $theme;

  /**
   * Any settings for this adapter.
   */
  protected $settings;


  /**
   * Constructor for the links.
   *
   * @param $type
   *   The name of the links plugin.
   */
  public function __construct($type, $args = NULL){
    global $theme_key;
    $this->type = $type;
    ctools_include('plugins');
    $plugin = ctools_get_plugins('sonar', 'adapter', $type);
    $this->plugin = $plugin;
    $this->theme = $theme_key;
    $this->scss = array();
    $this->id = '';
    $this->settings = variable_get('sonar_adapter_' . $plugin['name'], array()) + $this->settingsDefaults();
  }


  /**
   * If site is in live/production mode
   */
  protected function is_live(){
    return variable_get('sonar_production');
  }


  /**
   * Generate the unique ID used by SCSS files in this request.
   */
  protected function generateId(){
    // Create unique CID for these files
    $cid = implode('', array_keys($this->scss));
    $this->id = 'sonar-' . $this->theme . '-' . substr(drupal_hash_base64($cid), 0, 30);
  }


  /**
   * Take the SCSS files and operate on them.
   *
   * @param $scss
   *   An array of all SCSS items being requested on the page.
   */
  public function build($scss){
    $this->scss = $scss;
    $this->directory = sonar_get_destination() . '/' . $this->theme;
    $this->file = '';

    // Generate unique ID
    $this->generateId();

    // Unique file path
    $this->filepath = $this->directory . '/' . $this->id . '.css';

    // Make the magic happen
    $this->run();

    // Add our file to the page
    $css = drupal_add_css($this->filepath);
    $file = $css[$this->filepath];
    return $file;
  }


  /**
   * Validate our request and see if we need to compile.
   */
  protected function run(){
    // Compile if necessary.
    if($this->compileValidate() && $this->filesValidate()){
      try{
        $this->compilePrepare();
        $css = $this->compile();
        $this->compileComplete($css);
      }
      catch (Exception $e){
        drupal_set_message($e->getMessage(), 'error');
        return FALSE;
      }
    }
  }


  /**
   * Check to see if we need to compile.
   */
  protected function compileValidate(){

    // If the file doesn't exist, we need to compile.
    if(!file_exists($this->filepath)){
      return TRUE;
    }

    // Get cached data
    $cached = cache_get($this->id, 'cache_page');
    if(empty($cached->data)){
      return TRUE;
    }

    // If any files have been updated since last compile
    if(!$this->is_live()){
      foreach ($this->scss as $key => $file) {
        if($cached->data < filemtime($file['data'])){
          return TRUE;
        }
      }
    }

    return FALSE;
  }


  /**
   * Run directly before compiling.
   */
  protected function compilePrepare(){
    // Prepare files
    $this->filesPrepare();
    // Prepare destination directory
    $this->compilePrepareDirectory();
    // Create temporary file
    $this->compilePrepareTemp();
  }


  /**
   * Prepare destination directory.
   */
  protected function compilePrepareDirectory(){
    if (!file_prepare_directory($this->directory, FILE_CREATE_DIRECTORY)) {
      throw new Exception(t('The directory !directory could not be created or is not writable. Please consult your !watchdog for a detailed error description.', array('!directory' => $this->directory, '!watchdog' => l('log messages', 'admin/reports/dblog'))));
    };
  }


  /**
   * Create temporary SASS file.
   */
  protected function compilePrepareTemp(){
    $this->temp = $this->directory . '/' . 'tmp.' . $this->id . '.' . REQUEST_TIME . '.scss';
    if (!file_unmanaged_save_data($this->data, $this->temp, FILE_EXISTS_REPLACE)) {
      throw new Exception(t('The file <code>!file</code> could not be created. Please consult your !watchdog for a detailed error description.', array('!file' => $this->directory, '!watchdog' => l('log messages', 'admin/reports/dblog'))));
    };
  }

  /**
   * Compile SCSS files in CSS
   */
  protected function compile(){
    throw new Exception( 'No adapter currently being used.');
  }

  /**
   * Runs after compilation has completed successfully.
   *
   * @param $data
   *   A string of fully compiled CSS
   */
  protected function compileComplete($css){
    // Debug info
    if(sonar_is_debug()){
      watchdog('sonar', 'Compiled CSS: <textarea>!data</textarea>', array('!data' => $css), WATCHDOG_INFO, NULL);
    }

    // Remove temp file
    file_unmanaged_delete($this->temp);

    // Allow modifying of data before saving
    drupal_alter('sonar_css', $css);

    // Save the compiled data to final file location
    file_unmanaged_save_data($css, $this->filepath, FILE_EXISTS_REPLACE);

    // Set cache
    cache_set($this->id, time(), 'cache_page', CACHE_PERMANENT);
  }

  /**
   * Make sure we have a valid set of files.
   */
  protected function filesValidate(){
    $valid = TRUE;
    foreach ($this->scss as $key => $file){
      if (!file_exists($file['data'])) {
        drupal_set_message(t('Sonar is trying to use the file @file and it is not able to find it.', array('@file' => '<code>' . $key . '</code>')), 'error');
        $valid = FALSE;
      }
    }
    return $valid;
  }


  /**
   * Runs over all SCSS files to prepare the files for compilation.
   */
  protected function filesPrepare(){
    $this->data = array();
    foreach ($this->scss as $key => $file) {
      $this->data[] = $this->filesPrepareFile($file);
    }
    $this->filesClean();
  }


  /**
   * Prepare each SCSS file for aggregation. Each retured value gets added to
   * a single SCSS file that is then compiled using the selected adapter.
   */
  protected function filesPrepareFile($file){
    return '@import "' .DRUPAL_ROOT . '/' .$file['data'].'";'."\n";
  }


  /**
   * Make the data as small as possible but stripping out comments and other junk.
   */
  protected function filesClean(){
    // Combine into single string.
    $this->data = implode("\n", $this->data);

    /* remove comments */
    $this->data = preg_replace('!/\*[^*]*\*+([^/][^*]*\*+)*/!', '', $this->data);

    //  Removes multi-line comments and does not create
    //  a blank line, also treats white spaces/tabs
    $this->data = preg_replace('!^[ \t]*/\*.*?\*/[ \t]*[\r\n]!s', '', $this->data);

    //  Removes single line '//' comments, treats blank characters
    // $this->data = preg_replace('![ \t]*//.*[ \t]*[\r\n]!', '', $this->data);
    $this->data = preg_replace('!(^\/\/.*)!m','',$this->data);

    //  Strip blank lines
    $this->data = preg_replace("/(^[\r\n]*|[\r\n]+)[\s\t]*[\r\n]+/", "\n", $this->data);

    // Debug info
    if(sonar_is_debug()){
      watchdog('sonar', 'Prepared SCSS: <textarea>!data</textarea>', array('!data' => $this->data), WATCHDOG_INFO, NULL);
    }
  }


  /**
   * An array containing default values for the settings form fields.
   */
  public function settingsDefaults(){
    return array();
  }


  /**
   * Configuration settings made available on the Sonar settings page.
   *
   * @param $form
   *   An empty array.
   * @param $form_state
   *   The form state of the parent form.
   * @param $default_values
   *   An array containing the values currently saved to the var table.
   */
  public function settingsForm(&$form, &$form_state){}

}

/**
 * A null implementation of Sonar_Adapter.
 */
class Sonar_Adapter_Broken extends Sonar_Adapter_Abstract {

}
