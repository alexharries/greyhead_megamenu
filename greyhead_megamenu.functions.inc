<?php

/**
 * @file
 * greyhead_megamenu.functions.inc
 *
 * Provides functions which aren't Drupal hooks, menu callbacks, etc.
 */

/**
 * Returns a system settings form.
 *
 * @return mixed
 */
function greyhead_megamenu_admin_settings() {
  $form = array();

  // @TODO

  // Manage defaults for new megamenus.

  return system_settings_form($form);
}

/**
 * Returns the Add Megamenu form.
 *
 * @return mixed
 */
function greyhead_megamenu_admin_list_megamenus() {
  $render_array = array();

  $render_array['intro'] = array('#markup' => '<p class="summary">' . t('This page lists the Megamenus which have been configured.') . '</p>');

  foreach (greyhead_megamenu_megamenus_list() as $megamenu_id => $megamenu) {
    dpm($megamenu);

    $title = t('Menu ID: !id - menu: !menu', array('!id' => $megamenu_id, '!menu' => $megamenu->configuration['menu']['name']));

    if (user_access('edit greyhead megamenus')) {
      $markup = l($title, 'admin/structure/block/greyheadmegamenu/edit/' . $megamenu_id);
    }
    else {
      $markup = $title;
    }

    $render_array[$megamenu_id] = array('#markup' => $markup);
  }

  return $render_array;
}

/**
 * Returns the Add/Edit Megamenu form.
 *
 * @return mixed
 */
function greyhead_megamenu_admin_edit_megamenu($megamenu_id = NULL) {
  $form = array();

  // @TODO

  // Choose megamenu menu and machine name.

  return $form;
}

// @TODO: validate and submit functions here

// @TODO: create second step form to set options

/**
 * Builds the HTML for a Greyhead Megamenu block.
 *
 * @param $megamenu greyhead_megamenu A Megamenu object.
 *
 * @return array
 * @throws \Exception
 */
//function greyhead_megamenu_build_menu_block($megamenu) {
//  $return_array = array(
//    // The block title.
//    'subject' => $megamenu->configuration['block']['subject'],
//    // The block's content, as a renderableableable array.
//    'content' => array(
//      '#markup' => theme('greyhead_megamenu_megamenu', array(
//        'id' => $megamenu->configuration['megamenu']['id'],
//        'classes' => $megamenu->configuration['megamenu']['classes'],
//        'items' => $megamenu->menuitems_config_get(),
//      )),
//    ),
//  );
//
//  return $return_array;
//}

/**
 * Get the default block configuration for hook_block_view.
 */
function greyhead_megamenu_menus_default() {
  $blocks = array();

  // @TODO: remove this when the interface to create/edit/delete blocks is done.
  $blocks['main_menu'] = array(
    'info' => t('Main menu Megamenu'),
    'cache' => DRUPAL_CACHE_PER_ROLE,
    'status' => FALSE,
    'region' => 'navigation',
    'visibility' => BLOCK_VISIBILITY_NOTLISTED,
    // We implode an array of paths which we don't want to see this block
    // displayed on, for code readability.
//    'pages' => implode(array(
//      '<front>',
//      'user',
//      'user/*',
//      'users',
//      'users/*',
//    )),
  );

  return $blocks;
}

/**
 * Render and return the HTML output for a node.
 *
 * $column_options contains the column's 'options' array, which must
 * contain an 'nid' key with the node's ID.
 *
 * @param $column_options
 *
 * @return array A renderable array of a node.
 */
function greyhead_megamenu_column_content_node($column_options, $column_id, $menuitem, greyhead_megamenu $megamenu) {
  return node_view(node_load($column_options['nid']));
}

/**
 * Render and return the HTML output for a block.
 *
 * $column_options contains the column's 'options' array, which must
 * contain 'module' and 'delta' keys.
 *
 * @param $column_options
 *
 * @return array A renderable array of a block.
 */
function greyhead_megamenu_column_content_block($column_options, $column_id, $menuitem, greyhead_megamenu $megamenu) {
  $block = module_invoke($column_options['module'], 'block_view', $column_options['delta']);
  return $block['content'];
}

/**
 * Render and return the HTML output for a submenu.
 *
 * $column_options contains the column's 'options' array, which must
 * contain a 'columns_count' key.
 *
 * @param $column_options
 *
 * @return array A renderable array with the HTML for the menu.
 */
function greyhead_megamenu_column_content_submenu($column_options, $column_id, $menuitem, greyhead_megamenu $megamenu) {
  return greyhead_megamenu_submenu_output(greyhead_megamenu_column_content_submenu_build_tree($menuitem['id']), $column_options, $column_id, $menuitem, $megamenu);
}

/**
 * Get the sub-menu structure for a given mlid.
 *
 * Code stolen with thanks from http://stackoverflow.com/a/11615338
 *
 * @param $mlid
 *
 * @return mixed
 */
function greyhead_megamenu_column_content_submenu_build_tree($mlid) {
  $render_array = _greyhead_megamenu_menu_build_subtree($mlid);

  return $render_array;
}

/**
 * Get the entire menu tree, access-checked, below a certain mlid.
 *
 * This function builds and recurses down a menu tree until it finds $mlid, and
 * when it does, it returns the the link for mlid and any child links.
 *
 * Example return array:
 *
 *  array(
 *    'link' => a drupal menu link,
 *    'below' => menu tree array(
 *      '50000 Link 123' => array(
 *        'link' => a drupal menu link,
 *        'below' => menu tree array(
 *          ... and so on.
 *        ),
 *      ),
 *    ),
 *  );
 *
 * @param      $mlid
 * @param bool $only_children Set to TRUE if you only want the children of mlid,
 *                            of FALSE if you also want mlid.
 * @param null $menuitem_menu_link
 * @param null $menu_name
 * @param null $menulinks
 *
 * @return array An array containing a menu structure, if found, or an empty
 *               array.
 */
function _greyhead_megamenu_menu_build_subtree($mlid, $only_children = TRUE, $menuitem_menu_link = NULL, $menu_name = NULL, $menulinks = NULL) {
  if (is_null($menuitem_menu_link)) {
    $menuitem_menu_link = menu_link_load($mlid);
  }

  if (is_null($menu_name)) {
    $menu_name = $menuitem_menu_link['menu_name'];
  }

  if (is_null($menulinks)) {
    $menulinks = menu_build_tree($menuitem_menu_link['menu_name']);
  }

  foreach ($menulinks as $menulink_and_children) {
    if ($menulink_and_children['link']['mlid'] == $mlid) {
      if ($only_children) {
        if (isset($menulink_and_children['below']) && !empty($menulink_and_children['below'])) {
          return $menulink_and_children['below'];
        }
        return array();
      }
      return $menulink_and_children;
    }

    $twig = _greyhead_megamenu_menu_build_subtree($mlid, $only_children = TRUE, $menu_name, $menuitem_menu_link, $menulink_and_children['below']);

    if ($twig) {
      return $twig;
    }
  }

  return array();
}

/**
 * Converts a structured menu array from menu_build_tree into a render array.
 *
 * This function also changes the theme callback for the top level of any sub-
 * menu links to use theme_greyhead_megamenu_column_submenu.
 *
 * Note that functions ending in _output in this module generally return render
 * arrays.
 *
 * @see greyhead_megamenu_column_content_submenu_build_tree().
 *
 * @param $submenu
 *
 * @return array
 */
function greyhead_megamenu_submenu_output($submenu, $column_options, $column_id, $menuitem, greyhead_megamenu $megamenu) {
  $submenu_output = menu_tree_output($submenu);

  $render_array = array(
    '#id' => drupal_html_id('megamenu-' . $megamenu->id() . '-menuitem-' . $menuitem['id'] . '-submenu-column-' . $column_id . '-submenu'),
    '#theme_wrappers' => array(),
    '#theme' => 'greyhead_megamenu_column_submenu',
    '#columns' => array(),
  );

  // Change the theme function on the top-level menu items.
  foreach (element_children($submenu_output) as $submenu_item_key) {
    $submenu_output[$submenu_item_key]['#theme'] = 'greyhead_megamenu_column_submenu_column';
    $submenu_output[$submenu_item_key]['#theme_wrappers'] = array();
  }

//  dpm($render_array);

  /**
   * Attempt to split the submenu into the number of columns in
   * $column_options['columns_count'].
   *
   * This is only a best-guess a might be better-served through CSS and some JS
   * magic, but here's a rough attempt...
   *
   * Create a mappings array which maps submenus to columns.
   */
//  $column_options['mappings'] = array();
//  $column_options['submenu_items_count'] = ;

//  /**
//   * If there are fewer top-level items in this sub-menu than there are columns,
//   * we change the number of columns to equal the number of items.
//   */
//  if (count(element_children($submenu_output)) <= $column_options['columns_count']) {
//    $column_options['columns_count'] = $column_options['submenu_items_count'];
//
//    // Assign each submenu item to a column and we're done.
//    $column_number = 0;
//    foreach (element_children($submenu_output) as $submenu_item_key) {
//      $column_options['mappings'][$column_number] = array($submenu_item_key);
//      $column_number++;
//    }
//  }
  /**
   * Count the number of links in each menu and, based on this, try
   * to spread the submenus evenly across the columns.
   */
//  else {
  // Temporary: just split the submenu items evenly across the columns.
//  dpm($column_options);
  $submenus_per_column = ceil(count(element_children($submenu_output)) / $column_options['columns_count']);

  // Initialise our counters.
  $column_number = 0;
  $menuitem_number = 0;
  $total_menuitems = count(element_children($submenu_output));

  // Create a template render array.
  $template_column_render_array = array(
    '#theme' => 'greyhead_megamenu_column_submenu_column',
    '#items' => array(),
  );

  // And set the first column up.
  $render_array['#columns'][$column_number] = $template_column_render_array;

  /**
   * Now we know how many menu items per column. For example, if we have 9
   * menu items and 4 columns, we will have (9 / 4 rounded up = 3 per column):
   *
   * Column 0: menu items 1, 2, 3
   * Column 1: menu items 4, 5, 6
   * Column 2: menu items 7, 8, 9
   *
   * ... and we will leave the fourth column empty.
   */
  foreach (element_children($submenu_output) as $submenu_item_key) {
    $menuitem_number++;

    // Copy the menu item's render array.
    $render_array['#columns'][$column_number]['#items'][$submenu_item_key] = $submenu_output[$submenu_item_key];

    // In the example above, if $menuitem_number % $submenus_per_column == 0,
    // move to the next column. e.g. if we've just assigned the 4th menu item
    // and there are 4 items per column, then 4 / 4 gives a remainder of 0;
    // same with 8 / 4, so we increment the column number.
    if (($menuitem_number % $submenus_per_column == 0)
      && ($menuitem_number < $total_menuitems)
    ) {
      $column_number++;
      $render_array['#columns'][$column_number] = $template_column_render_array;
    }
  }

  /**
   * Work out the column class. This is floor(12 / number of submenu
   * columns) with the remainder spread eveny across the first however-many
   * columns.
   *
   * Here's some examples (because I need to work this through in my head
   * before I bash out any more crufty code...):
   *
   * Submenu columns 1: column class 12 (full width).
   * 2 columns: column class 6 (50% each).
   * 3 columns: column class 4 (1/3 each).
   * 4 columns: column class 3 (1/4 each).
   * 5 columns: ick. column class floor(12 / 5) = 2 + 1 remainder, 2 + 1 remainder, 2 + 0 remainder, 2 + 0 remainder, 2 + 0 remainder = 12
   * 6 columns: column class 2 (1/6 each).
   * 7 columns: column class floor(12 / 7) = 1 + 1 remainder, 1 + 1 remainder, 1 + 1 remainder, 1 + 1 remainder, 1 + 1 remainder, 1, 1, 1, 1, 1, 1, 1 = 12
   */
  $columns_count = count(element_children($render_array['#columns']));
  $columns_per_column = floor(12 / $columns_count);
  $remainder = 12 % $columns_count;
  $remainder_to_add_to_this_column = 0;

  foreach ($render_array['#columns'] as $column_number => &$column) {
    // Do we have any remainder left to add to this column?
    if ($remainder > 0) {
      $remainder_to_add_to_this_column = 1;
      $remainder--;
    }

    // Set the class.
    $column['#column_class'] = 'col-sm-' . ($columns_per_column + $remainder_to_add_to_this_column);
  }

  // Now we know how many columns have been filled, we can update the column
  // class.
//  $render_array['#column_class'] = 'col-sm-' . (12 / count($render_array['columns']));

//    drupal_set_message(__FUNCTION__ . ' split submenu columns better - @TODO');
//  }

//  dpm($render_array);

  return $render_array;
}

/**
 * Gets or sets the list of configured megamenus, and saves the list to the
 * database when called with $do_save = TRUE.
 *
 * @param null       $updated_list
 * @param bool|FALSE $do_save
 *
 * @return array|bool|null
 */
function greyhead_megamenu_megamenus_list($updated_list = NULL, $do_save = FALSE) {
  $megamenus_list = &drupal_static(__FUNCTION__ . ':list');
  $megamenus_needs_save = &drupal_static(__FUNCTION__ . ':needs_save', FALSE);

  // If the megamenus list hasn't already been loaded into memory, load it.
  if (is_null($megamenus_list)) {
    $megamenus_list = variable_get('greyhead_megamenu_menus');

    // If no list was available in the database, use defaults and flag for
    // saving in the database.
    if (is_null($megamenus_list)) {
      $megamenus_list = greyhead_megamenu_menus_default();
      $megamenus_needs_save = TRUE;
    }
  }

  // Has an updated list been passed in for saving?
  if (!is_null($updated_list) && is_array($updated_list)) {
    $megamenus_list = $updated_list;
    $megamenus_needs_save = TRUE;
    return TRUE;
  }

  // Have we been asked to save the data to the database?
  if ($do_save) {
    $megamenus_needs_save = FALSE;
    variable_set('greyhead_megamenu_menus', $updated_list);
    return TRUE;
  }

  // Otherwise, just return the current list.
  return $megamenus_list;
}

/**
 * Get a list of loaded megamenus, or store a megamenu object in memory.
 *
 * This function keeps a track of megamenus which have been loaded, and is used
 * on drupal_exit to write any changes to the megamenus to the database.
 *
 * @param null|greyhead_megamenu $megamenu_to_add
 *
 * @return greyhead_megamenu|void If $megamenu_to_add is provided, nothing will
 *                                be returned; if $megamenu_to_add is not
 *                                provided, an array of loaded megamenus is
 *                                returned, with the megamenu's ID as the key.
 */
function greyhead_megamenu_menus_loaded($megamenu_to_add = NULL) {
  $megamenus_loaded = &drupal_static(__FUNCTION__, array());

  if (!is_null($megamenu_to_add) && $megamenu_to_add->id()) {
    $megamenus_loaded[$megamenu_to_add->id()] = $megamenu_to_add;
  }
  else {
    return $megamenus_loaded;
  }
}

/**
 * Load function for hook_menu to get a megamenu object.
 *
 * @param $megamenu_id
 *
 * @return bool|\greyhead_megamenu A megamenu object if found, or FALSE if not.
 */
function greyhead_megamenu_load($megamenu_id) {
  if ($megamenu = new greyhead_megamenu($megamenu_id, FALSE)) {
    return $megamenu;
  }

  return FALSE;
}
