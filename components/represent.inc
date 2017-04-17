<?php

/**
 * @file
 *   Represent module Represent component.
 */

/**
 * Specify the default properties of a component.
 *
 * @return
 *   An array defining the default structure of a component.
 */
// Define component and its basic capabilities

function _webform_defaults_represent() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'extra' => array(
      'title_display' => 0,
      'description' => '',
      'private' => FALSE,
      // Represent specific configuration
      'options_source' => array(),
      'postal_code_component' => '',
      'multiple' => 0,
      'autoselect' => 0,
      'hide' => 0,
      // Quack like a select component.
      'aslist' => 0,
      'multiple' => 1,
      'optrand' => 0,

      'other_option' => 0,
    ),
  );
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_represent($component, $value, $format = 'html') {
  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_represent',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#format' => $format,
    '#value' => _webform_represent_values_to_string($value),
    '#translatable' => array('title'),
  );
}

/**
 * Generate the form for editing a component.
 *
 * Create a set of form elements to be displayed on the form for editing this
 * component. Use care naming the form items, as this correlates directly to the
 * database schema. The component "Name" and "Description" fields are added to
 * every component type and are not necessary to specify here (although they
 * may be overridden if desired).
 *
 * @param $component
 *   A Webform component array.
 * @return
 *   An array of form items to be displayed on the edit component page.
 */
function _webform_edit_represent($component) {
  $form = array();
  module_load_include('inc', 'webform_represent', 'includes/webform_represent.options');

  $form['represent_extra'] =  array(
    '#type' => 'fieldset',
    '#title' => t('Representatives Configuration'),
    '#weight' => 1,
    '#collapsible' => TRUE,
  );

  $form['represent_extra']['options_source'] = array(
    '#title'         => t('Representative set'),
    '#type'          => 'select',
    '#options'       => webform_represent_options_source(),
    '#default_value' => $component['extra']['options_source'],
    '#description'   => t("The elected official from this representative set whose electoral district intersects the supporter's postal code will receive a copy of the submission."),
    '#required'      => TRUE,
    '#multiple'      => TRUE,
    '#size'          => 4,
    '#parents' => array('extra', 'options_source'),
    '#weight' => 0,
  );

  $node = node_load($component['nid']);
  $options = array('' => t('- Select -'));
  if (!empty($node->webform['components'])) {
    foreach ($node->webform['components'] as $array) {
      switch ($array['type']) {
        case 'postal_code':
          $options[$array['cid']] = $array['name'];
          break;
      }
    }
  }

  $form['represent_extra']['postal_code_component'] = array(
    '#title'         => t('Postal code component'),
    '#type'          => 'select',
    '#options'       => $options,
    '#default_value' => $component['extra']['postal_code_component'],
    '#description'   => t('If you have not yet created a postal code component, go back and create one.'),
    '#required'      => TRUE,
    '#parents' => array('extra', 'postal_code_component'),
    '#weight' => 1,
  );

  $form['represent_extra']['multiple'] = array(
    '#title'         => t('Allow Multiple Selections'),
    '#type'          => 'checkbox',
    '#default_value' => $component['extra']['multiple'],
    '#description'   => t('Enabling this option will allow multiple representatives to be selected. Leaving this option unchecked will limit selection to only one representative'),
    '#parents' => array('extra', 'multiple'),
    '#weight' => 2,
  );

  $form['represent_extra']['autoselect'] = array(
    '#title'         => t('Auto-Select'),
    '#type'          => 'checkbox',
    '#default_value' => $component['extra']['autoselect'] || $component['extra']['autoselect_hide'],
    '#description'   => t('Enabling this option will automatically select the matching representative(s). If "Allow Multiple Selections" is enabled, all representatives will be selected.  If "Allow Mutiple Selections" is disabled, the first matching representative will be auto-selected.'),
    '#parents' => array('extra', 'autoselect'),
    '#weight' => 3,
  );

  $form['represent_extra']['hide'] = array(
    '#title'         => t('Hide'),
    '#type'          => 'checkbox',
    '#default_value' => $component['extra']['hide'] || $component['extra']['autoselect_hide'],
    '#description'   => t('Enabling this option will automatically hide the component from the user. This option requires that "Auto-Select" is disabled.'),
    '#parents' => array('extra', 'hide'),
    '#weight' => 5,
  );

  return $form;
}

/*
 *
 */
function _webform_render_represent($component, $value = NULL, $filter = TRUE) {
  $node = isset($component['nid']) ? node_load($component['nid']) : NULL;

  $widget_type = 'checkboxes';
  if (!$component['extra']['multiple']) {
    $widget_type = 'radios';
  }
  $element = array(
    '#type' => $widget_type,
    '#options' => array(),
    '#title' => $filter ? _webform_filter_xss($component['name']) : $component['name'],
    '#title_display' => $component['extra']['title_display'] ? $component['extra']['title_display'] : 'before',
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#theme_wrappers' => array('webform_element'),
    '#translatable' => array('title', 'description'),
    '#pre_render' => array('webform_represent_pre_render_cleanup'),
  );
  if (isset($value) && $value) {
    $element['#options'] = $value;
    if ($component['extra']['multiple']) {
      $element['#default_value'] = $value;
    }
    else {
      $element['#default_value'] = current($value);
    }
  }

  // Handle disabling.
  if (isset($component['extra']['disabled']) && $component['extra']['disabled']) {
    if ($filter) {
      $element['#attributes']['readonly'] = 'readonly';
    }
    else {
      $element['#disabled'] = TRUE;
    }
  }

  return $element;
}

/**
 * Implements _webform_analysis_component().
 */
function _webform_analysis_represent($component, $sids = array()) {
  $query = db_select('webform_submitted_data', 'wsd', array('fetch' => PDO::FETCH_ASSOC))
      ->fields('wsd', array('data'))
      ->condition('nid', $component['nid'])
      ->condition('cid', $component['cid']);

  if (count($sids)) {
    $query->condition('sid', $sids, 'IN');
  }

  $nonblanks = 0;
  $submissions = 0;

  // This could possibly be extended to count submissions by
  // country using the postal_code_validation API.
  $result = $query->execute();
  foreach ($result as $data) {
    if (drupal_strlen(trim($data['data'])) > 0) {
      $nonblanks++;
    }
    $submissions++;
  }

  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User entered value'), $nonblanks);
  return $rows;
}

/**
 * Implements _webform_csv_headers_component().
 */
function _webform_csv_headers_represent($component, $export_options) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2][] = t('@name Email', array('@name' => $component['name']));
  return $header;
}

/**
 * Implements _webform_csv_data_component().
 */
function _webform_csv_data_represent($component, $export_options, $value) {
  return _webform_represent_values_to_string($value);
}

/**
 * Implements _webform_table_component().
 */
function _webform_table_represent($component, $value) {
  return _webform_represent_values_to_string($value);
}

/**
 * Implements _webform_theme_component().
 */
function _webform_theme_represent() {
  return array(
    'webform_display_represent' => array(
      'render element' => 'element',
    ),
  );
}

/*
 * We do not need to define:
 * - _webform_attachments_component
 * - _webform_delete_component
 * - _webform_help_component
 * - _webform_submit_component
 */


/**
 * Creates string from array of Represent values.
 */
function _webform_represent_values_to_string($value) {
  $output = array();
  foreach ($value as $key => $contained) {
    if ($contained) {
      if ($resource = webform_represent_resource_load('', $contained)) {
        $output[] = webform_represent_format_resource($resource);
      }
      else {
        $output[] = $contained;
      }
    }
  }
  return check_plain(empty($output) ? '' : implode(', ', $output));
}
