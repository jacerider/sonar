<?php

/**
 * Menu callback for settings form.
 */
function sonar_settings($form, $form_state) {

  $form['sonar_sass'] = array(
    '#type' => 'fieldset',
    '#title' => t('Compass (SCSS) settings'),
  );
  $form['sonar_sass']['sonar_sass'] = array(
    '#type' => 'checkbox',
    '#title' => t('Compile SCSS to CSS'),
    '#attributes' => array(
      'class' => array('enable-extension'),
    ),
    '#description' => t('Sass integration - uncheck if you are already using a different sass compiler.'),
    '#element_validate' => array('sonar_sass_validate'),
    '#default_value' => variable_get('sonar_sass'),
  );
  $form['sonar_sass']['sonar_remote'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use remote compiling'),
    '#description' => t('Sass integration - uncheck if you are already using a different sass compiler.'),
    '#default_value' => isset($_SERVER['PANTHEON_ENVIRONMENT']) ? 1 : variable_get('sonar_remote'),
    '#disabled' => isset($_SERVER['PANTHEON_ENVIRONMENT']),
  );

  $form['sonar_sass']['sonar_compass_path'] = array(
    '#type' => 'fieldset',
    '#title' => t('Compass Path'),
    '#states' => array(
      'visible' => array(   // action to take.
        ':input[name="sonar_remote"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['sonar_sass']['sonar_compass_path']['description'] = array(
    '#markup' => '<div class="description">' . 'Set the path to where your compass compiler is installed. To find out what this is, you can open up your console and type "which compass".' . '</div>',
  );
  $form['sonar_sass']['sonar_compass_path']['sonar_compass_path'] = array(
    '#type' => 'textfield',
    '#attributes' => array(
      'placeholder' => t('e.g.') . ' /usr/bin/compass',
    ),
    '#default_value' => variable_get('sonar_compass_path'),
  );

  sonar_compass_exec('version', $output, $errors);
  $output = empty($output) ? t('Compass not found. Please update executable path.') : $output;
  $form['sonar_sass']['sonar_compass_path']['version'] = array(
    '#type' => 'item',
    '#title' => t('Version information'),
    '#markup' => '<pre>' . check_plain(trim($output)) . '</pre>',
  );

  $files_directory = variable_get('file_' . file_default_scheme() . '_path', conf_path() . '/files') . '/css';
  $form['sonar_sass']['sonar_compiled_path'] = array(
    '#type' => 'fieldset',
    '#title' => t('Compiled files path'),
  );
  $form['sonar_sass']['sonar_compiled_path']['description'] = array(
    '#markup' => '<div class="description">' . t('Set the path to where you would like compiled files to be stored. defaults to <code>!files</code>', array('!files' => $files_directory)) . '<br />' .
    t('Compiled files will be stored in a sub-directory with the theme name so entering the path to your themes directory here will place the copiled files in a <code>/stylesheets/</code> directory under each theme\'s directory.') . '</div>',
  );
  $form['sonar_sass']['sonar_compiled_path']['sonar_compiler_destination'] = array(
    '#type' => 'textfield',
    '#attributes' => array(
      'placeholder' => t('e.g.') . ' sites/all/themes/css',
    ),
    '#default_value' => variable_get('sonar_compiler_destination'),
  );

  $form['sonar_sass']['sonar_auth'] = array(
    '#type' => 'fieldset',
    '#title' => t('Authentification'),
    '#description' => t('If you are using remote compiling and your server requires authentification you can use the below fields to enter your authentification credentials.'),
    '#states' => array(
      'visible' => array(   // action to take.
        ':input[name="sonar_remote"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['sonar_sass']['sonar_auth']['sonar_auth_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => variable_get('sonar_auth_user'),
  );
  $form['sonar_sass']['sonar_auth']['sonar_auth_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('sonar_auth_pass'),
  );

  $form['sonar_sass']['sonar_url_helpers'] = array(
    '#type' => 'fieldset',
    '#title' => t('Compass URL Helpers'),
  );
  $form['sonar_sass']['sonar_url_helpers']['description'] = array(
    '#markup' => '<div class="description">' . t('Here you may set the paths to be used with !helperslink, this allows you to easily restructure your theme or change them to be using asset hosts when moving to production.', array('!helperslink' => '<a target="_blank" href="http://compass-style.org/reference/compass/helpers/urls/">' . t('Compass URL helper functions') . '</a>')) . ' <strong>' . t('if none is set these functions will point to your theme\'s /images, /fonts and /styles directories.') . '</strong></div>',
  );
  $form['sonar_sass']['sonar_url_helpers']['sonar_images_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to images directory'),
    '#description' => t('Set the path to your images, this may be used with the !imageurl helper function. Default path is %default.', array('!imageurl' => '<a target="_blank" href="http://compass-style.org/reference/compass/helpers/urls/#image-url">image-url(..)</a>', '%default' => '[CURRENT THEME]/assets/images')),
    '#attributes' => array(
      'placeholder' => t('e.g. /path/to/images or http://yourhost.com/path/to/images'),
    ),
    '#default_value' => variable_get('sonar_images_path'),
  );
  $form['sonar_sass']['sonar_url_helpers']['sonar_fonts_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to fonts directory'),
    '#description' => t('Set the path to your fonts, this may be used with the !fontsurl helper function. Default path is %default.', array('!fontsurl' => '<a target="_blank" href="http://compass-style.org/reference/compass/helpers/urls/#font-url">font-url(..)</a>', '%default' => '[CURRENT THEME]/assets/fonts')),
    '#attributes' => array(
      'placeholder' => t('e.g. /path/to/fonts or http://yourhost.com/path/to/fonts'),
    ),
    '#default_value' => variable_get('sonar_fonts_path'),
  );

  $form['sonar_sass']['sonar_debug'] = array(
    '#type' => 'fieldset',
    '#title' => t('Debugging'),
  );
  $form['sonar_sass']['sonar_debug']['sonar_devel'] = array(
    '#type' => 'checkbox',
    '#title' => t('Development mode - unminified output and FireSass support.'),
    '#description' => t('Sass Development - Output unminified sass for better readability and add !firesass support. <strong>Note:</strong> css output is way bigger, use only while in development.', array('!firesass' => '<a target="blank" href="https://addons.mozilla.org/en-US/firefox/addon/firesass-for-firebug/">FireSass</a>')),
    '#default_value' => variable_get('sonar_devel'),
  );

  return system_settings_form($form);
}

/**
 * Flush all CSS and page caches so Sass files are recompiled.
 *
 * @see _admin_menu_flush_cache()
 */
function sonar_sass_validate($element, &$form_state) {
  if ($element['#value']) {
    // Change query-strings on css/js files to enforce reload for all users.
    _drupal_flush_css_js();

    drupal_clear_css_cache();
    drupal_clear_js_cache();

    // Clear the page cache, since cached HTML pages might link to old CSS and
    // JS aggregates.
    cache_clear_all('*', 'cache_page', TRUE);
    drupal_set_message(t('Your Sass files will be recompiled'), 'status');
  }
}

/**
 * Execute compass commands
 */
function sonar_compass_exec($command, &$output, &$errors) {
  $compass_path = variable_get('sonar_compass_path', '/usr/bin/compass');
  $errors = sonar_compass_check_path($compass_path);
  if ($errors) {
    watchdog('sonar', '!errors', array('!errors' => implode('<br />', $errors)), WATCHDOG_ERROR);
    return FALSE;
  }

  // Specify Drupal's root as the working a working directory so that relative
  // paths are interpreted correctly.
  $drupal_path = $_SERVER['DOCUMENT_ROOT'] . dirname($_SERVER['SCRIPT_NAME'] ? $_SERVER['SCRIPT_NAME'] : $_SERVER['SCRIPT_FILENAME']);

  $descriptors = array(
    0 => array('pipe', 'r'), // stdin
    1 => array('pipe', 'w'), // stdout
    2 => array('pipe', 'w'), // stderr
  );

  $env = array();
  $env['HOME'] = '/';

  if ($h = proc_open($compass_path . ' ' . $command, $descriptors, $pipes, $drupal_path, $env)) {
    stream_set_blocking($pipes[1], 0);
    stream_set_blocking($pipes[2], 0);

    while (!feof($pipes[1]) || !feof($pipes[2])) {
      if (!feof($pipes[1])) {
        $output .= stream_get_contents($pipes[1]);
      }
      if (!feof($pipes[2])) {
        $out = stream_get_contents($pipes[2]);
        if(!empty($out)) $errors .= $out;
      }
    }

    fclose($pipes[0]);
    fclose($pipes[1]);
    fclose($pipes[2]);
    $return_code = proc_close($h);

    if ($return_code != 0) {
      // If Compass returned a non-zero code, trigger a PHP error that will
      // be caught by Drupal's error handler, logged to the watchdog and
      // eventually displayed to the user if configured to do so.

      // If $errors is empty, only report the error code.
      if (empty($errors)) {
        trigger_error(t('Compass reported error code !code.', array('!code' => $return_code)), E_USER_ERROR);
      }
      // Otherwise report the error code, and the error message.
      else {
        trigger_error(t("Compass reported error code !code.\nMessage:\n!error", array('!code' => $return_code, '!error' => $errors)), E_USER_ERROR);
      }
    }

    return $return_code;
  }
  return FALSE;
}

function sonar_compass_check_path($path) {
  $errors = array();
  if (!is_file($path)) {
    $errors[] = t('The specified Compass path %file does not exist.', array('%file' => $path));
  }
  if (!$errors && !is_executable($path)) {
    $errors[] = t('The specified Compass path %file is not executable.', array('%file' => $path));
  }
  if ($errors && $open_basedir = ini_get('open_basedir')) {
    $errors[] = t('PHP\'s <a href="!open-basedir">open_basedir</a> security restriction is set to %open-basedir, which may be interfering with attempts to locate Compass.', array('%file' => $path, '%open-basedir' => $open_basedir, '!info-link' => url('http://php.net/features.safe-mode#ini.open-basedir')));
  }
  return $errors;
}
