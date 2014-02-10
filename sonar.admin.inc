<?php

/**
 * Menu callback for settings form.
 */
function sonar_settings($form, $form_state) {

  drupal_add_css(drupal_get_path('module','sonar') . '/css/sonar.admin.css');
  drupal_add_css(drupal_get_path('module','sonar') . '/css/sonar.admin.scss');

  $form['sonar'] = array(
    '#type' => 'markup',
    '#markup' => '<div id="sonar">' . t('If Sonar is working correctly, you will see the Sonar logo in this location.') . '</div>',
  );

  $form['sonar_status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Sonar'),
    '#description' => t('If enabled, all .scss file on your site will be processed with Sonar.'),
    '#default_value' => sonar_is_enabled(),
  );

  $form['sonar_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sonar Settings'),
    '#states' => array(
      'visible' => array(   // action to take.
        ':input[name="sonar_status"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['sonar_settings']['sonar_production'] = array(
    '#type' => 'checkbox',
    '#title' => t('Production mode'),
    '#description' => t('Enable when site is in production to increase performance.'),
    '#default_value' => variable_get('sonar_production'),
  );

  $form['sonar_settings']['sonar_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable debugging'),
    '#description' => t('Use to help troubleshoot and log Sonar events.'),
    '#default_value' => variable_get('sonar_debug'),
  );

  $options = array();
  foreach(ctools_get_plugins('sonar', 'adapter') as $plugin){
    $options[$plugin['name']] = $plugin['title'];
  }
  asort($options);
  $form['sonar_settings']['sonar_adapter'] = array(
    '#type' => 'select',
    '#title' => t('Adapter'),
    '#description' => t('The SASS adapter to use to compile your SASS files.'),
    '#options' => $options,
    '#default_value' => sonar_active_adapter(),
  );

  foreach(ctools_get_plugins('sonar', 'adapter') as $plugin){
    $handler = sonar_get_handler($plugin['plugin type'], $plugin['name']);
    $temp_form = array();
    $handler->settingsForm($temp_form, $form_state);
    if(!empty($temp_form)){
      $form['sonar_settings']['sonar_adapter_' . $plugin['name']] = array(
        '#type' => 'fieldset',
        '#title' => t($plugin['title']) . ' ' . t('Settings'),
        '#tree' => TRUE,
        '#states' => array(
          'visible' => array(   // action to take.
            ':input[name="sonar_adapter"]' => array('value' => $plugin['name']),
          ),
        ),
      ) + $temp_form;
    }
  }

  $files_directory = variable_get('file_' . file_default_scheme() . '_path', conf_path() . '/files') . '/sonar';
  $form['sonar_settings']['sonar_destination'] = array(
    '#type' => 'fieldset',
    '#title' => t('Compiled files path'),
  );
  $form['sonar_settings']['sonar_destination']['description'] = array(
    '#markup' => '<div class="description">' . t('Set the path to where you would like compiled files to be stored. Defaults to <code>!files</code>.', array('!files' => $files_directory)) . '</div>',
  );
  $form['sonar_settings']['sonar_destination']['sonar_destination'] = array(
    '#type' => 'textfield',
    '#attributes' => array(
      'placeholder' => t('e.g.') . ' ' . $files_directory,
    ),
    '#default_value' => variable_get('sonar_destination'),
  );

  $form = system_settings_form($form);
  array_unshift($form['#submit'], 'sonar_settings_submit');
  return $form;
}

/**
 * Settings submit callback.
 */
function sonar_settings_submit($form, &$form_state){
  $values = &$form_state['values'];
  if(empty($values['sonar_status'])){
    unset($values['sonar_settings']);
    db_query("DELETE FROM {variable} WHERE name LIKE 'sonar_%'");
  }

  // Clean up adapter settings from the var table.
  foreach(ctools_get_plugins('sonar', 'adapter') as $plugin){
    if(isset($values['sonar_adapter']) && $values['sonar_adapter'] == $plugin['name']){
      continue;
    }
    unset($values['sonar_adapter_' . $plugin['name']]);
    variable_del('sonar_adapter_' . $plugin['name']);
  }
}
