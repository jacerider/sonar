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

/**
 * Allow variables to be injected into the SASS files on compile.
 *
 *  - 'group': A number identifying the group in which to add the stylesheet.
 *     Available constants are:
 *     - CSS_SYSTEM: Any system-layer CSS.
 *     - CSS_DEFAULT: (default) Any module-layer CSS.
 *     - CSS_THEME: Any theme-layer CSS.
 *     The group number serves as a weight: the markup for loading a stylesheet
 *     within a lower weight group is output to the page before the markup for
 *     loading a stylesheet within a higher weight group, so CSS within higher
 *     weight groups take precendence over CSS within lower weight groups.
 *   - 'every_page': For optimal front-end performance when aggregation is
 *     enabled, this should be set to TRUE if the stylesheet is present on every
 *     page of the website for users for whom it is present at all. This
 *     defaults to FALSE. It is set to TRUE for stylesheets added via module and
 *     theme .info files. Modules that add stylesheets within hook_init()
 *     implementations, or from other code that ensures that the stylesheet is
 *     added to all website pages, should also set this flag to TRUE. All
 *     stylesheets within the same group that have the 'every_page' flag set to
 *     TRUE and do not have 'preprocess' set to FALSE are aggregated together
 *     into a single aggregate file, and that aggregate file can be reused
 *     across a user's entire site visit, leading to faster navigation between
 *     pages. However, stylesheets that are only needed on pages less frequently
 *     visited, can be added by code that only runs for those particular pages,
 *     and that code should not set the 'every_page' flag. This minimizes the
 *     size of the aggregate file that the user needs to download when first
 *     visiting the website. Stylesheets without the 'every_page' flag are
 *     aggregated into a separate aggregate file. This other aggregate file is
 *     likely to change from page to page, and each new aggregate file needs to
 *     be downloaded when first encountered, so it should be kept relatively
 *     small by ensuring that most commonly needed stylesheets are added to
 *     every page.
 */
function hook_sonar_var(){
  return array(
    // Will be injected as '$variable_name: variable_value;' before all
    // DEFAULT_CSS grouped files.
    'variable_name' => 'variable_value',

    // Will be injected as '$variable_name: true;' before all
    // CSS_DEFAULT grouped files.
    'variable_name' => array(
      'value' => TRUE,
      'group' => CSS_DEFAULT,
    ),

    // Will be injected as '$variable_name: true;' before all
    // CSS_SYSTEM grouped files.
    'variable_name' => array(
      'value' => TRUE,
      'group' => CSS_SYSTEM,
    ),

    // Will be injected as '$variable_name: true;' before all
    // CSS_THEME grouped files.
    'variable_name' => array(
      'value' => TRUE,
      'group' => CSS_THEME,
    ),
  );
}

/**
 * Perform necessary alterations to the injected SASS variables before they are
 * compiled.
 *
 * @param $vars
 *   An array of all variables being injected into SCSS.
 *
 * @see sonar_vars()
 */
function hook_sonar_var_alter(&$vars){

}
