<?php

/**
 * @file
 * API documentation file for Sonar.
 */

/**
 * Allows modules to alter the compiled CSS.
 *
 * This hook allow the altering of the CSS after it has been compiled by the
 * adapter.
 *
 * @param $css
 *  The compiled CSS that will be saved to file.
 *
 * @see Sonar_Adapter_Abstract.compileComplete()
 */
function hook_sonar_css_alter(&$css) {
  // Font Awesome path replace
  $css = str_replace('../fonts/', url(drupal_get_path('module', 'fawesome').'/fonts/'), $css);
}
