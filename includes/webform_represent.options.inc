<?php

/**
 * @file
 *   A collection of built-in select list options for Webform Represent.
 */

/**
 * Option list containing representative sets
 */
function webform_represent_options_source(){//$component, $flat, $arguments) {
  webform_component_include('select');
  $options = array('' => t('- Select -'));
  $names = webform_represent_representative_sets();
  foreach ($names as $machine_name => $representative_set) {
    $options[$machine_name] = $representative_set->name;
  }
  asort($options);
  return $options;
}

/**
 * Option list containing the representatives in a representative set.
 */
function webform_represent_options_representatives($component, $flat, $filter, $arguments) {
  $options = array();

  $without_district = preg_match('/-without-district$/', $arguments);
  if ($without_district) {
    $arguments = preg_replace('/-without-district$/', '', $arguments);
  }

  $resources = represent_representatives_by_set($arguments);
  if (!empty($resources)) {
    foreach ($resources as $resource) {
      if (isset($resource->email)) {
        // Skips mayors.
        if (!empty($resource->district_name)) {
          $name = '';
          if (isset($resource->extra) && isset($resource->extra->honorific_prefix) && in_array($resource->extra->honorific_prefix, array('Hon.', 'Right Hon.'))) {
            $name .= "{$resource->extra->honorific_prefix} ";
          }
          $name .= $resource->name;
          if (!$without_district) {
            $name .= " ({$resource->district_name})";
          }
          $options[$resource->email] = $name;
        }
      }
    }

    // Remove representatives with non-unique emails.
    foreach (array_count_values(array_keys($options)) as $email => $count) {
      if ($count > 1) {
        unset($options[$email]);

        watchdog('Represent API', 'Duplicate email "@email" in set "@set"', array(
          '@email' => $email,
          '@set' => $arguments,
        ), WATCHDOG_WARNING);
      }
    }

    uasort($options, 'webform_represent_compare_name');
  }

  return $options;
}

/**
 * Sorts names after removing honorific prefixes.
 *
 * @param string $a
 *   A name.
 * @param string $b
 *   A name.
 * @return integer
 *   -1 if the first name should appear before the second name, 0 if the names
 *   are identical, and 1 if the first name should appear after the second name.
 */
function webform_represent_compare_name($a, $b) {
  $a = webform_represent_remove_honorific($a);
  $b = webform_represent_remove_honorific($b);
  return $a == $b ? 0 : ($a < $b ? -1 : 1);
}

/**
 * Removes the honorific prefix from the name.
 *
 * @param string $name
 *   A name.
 * @return string
 *   The name without any honorific prefix.
 */
function webform_represent_remove_honorific($name) {
  return preg_replace('/^(Dr\.|M(\.|iss|lle|me|rs?\.|s\.)|(Right )?Hon\.) /', '', $name);
}
