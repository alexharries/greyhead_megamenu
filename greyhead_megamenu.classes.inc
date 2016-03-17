<?php

/**
 * @file
 * greyhead_megamenu.classes.inc
 *
 * Provides class functionality for Greyhead Megamenu.
 */
class greyhead_megamenu {
  // This is the main configuration store. @TODO: make private and use only an API to access.
  public $configuration = array();

  // Set to TRUE when this Megamenu has changes which haven't yet been saved to
  // the database.
  private $save_pending = FALSE;

  /**
   * @param $block_delta
   */
  function __construct($block_delta, $create_if_not_found = TRUE) {
    // Set the megamenu's ID.
    $this->id($block_delta);

    // Try to load it; if that doesn't work, create it.
    if (!$this->load() && $create_if_not_found) {
      $this->create();
    }
  }

  /**
   * Get the ID of a Megamenu - this is also the block delta.
   *
   * @param null $block_delta
   *
   * @return bool
   */
  function id($block_delta = NULL) {
    if (is_null($block_delta)) {
      if (isset($this->configuration['megamenu']['id'])) {
        return $this->configuration['megamenu']['id'];
      }

      drupal_set_message(t('Attemped to get Megamenu\'s ID but none has been specified yet.'), 'error');
      return FALSE;
    }
    else {
      $this->configuration['megamenu']['id'] = $block_delta;
    }
  }

  /**
   * Loads a megamenu from the database, by its id.
   *
   * If the id is unknown, a new Megamenu is created.
   *
   * @return bool
   */
  function load() {
    if (is_null($this->id())) {
//      drupal_set_message(t('Attemped to load Megamenu block @block_delta without a block delta set.', array('@block_delta' => $this->id())), 'error');
      return FALSE;
    }

    $loaded_configuration = variable_get($this->get_config_variable_name($this->id()));

    // Is the block known?
    if (!is_null($loaded_configuration)) {
      // Not a new block, so reset any save_pending flag.
      $this->save_pending(FALSE);

      // Set the configuration.
      $this->configuration = $loaded_configuration;

      // Add the loaded megamenu to the list of loaded menus.
      greyhead_megamenu_menus_loaded($this);

      return TRUE;
    }
    else {
//      drupal_set_message(t('Attemped to load unknown Megamenu block @block_delta.', array('@block_delta' => $this->id())), 'error');
      return FALSE;
    }
  }

  /**
   * Converts a Drupal block delta into a menu ID which can be
   * @return string
   * @internal param $block_delta
   */
  function get_config_variable_name() {
    return 'greyhead_megamenu__megamenu__' . $this->id();
  }

  /**
   * Get or set a flag to indicate whether the Megamenu needs to be saved.
   *
   * @param null $save_pending
   *
   * @return bool
   */
  function save_pending($save_pending = NULL) {
    if (is_null($save_pending)) {
      return $this->save_pending;
    }
    else {
      $this->save_pending = $save_pending;
      if ($save_pending) {
//        dpm($this->configuration);
      }
    }
  }

  /**
   * Initialises the current Megamenu as a new object, if its id isn't known.
   *
   * @return bool
   */
  function create() {
    if (is_null($this->id())) {
//      drupal_set_message(t('Attemped to initialise a new Megamenu block without a block delta set.'), 'error');
      return FALSE;
    }

    $loaded_configuration = variable_get($this->get_config_variable_name($this->id()));

    if (!is_null($loaded_configuration)) {
//      drupal_set_message(t('Attemped to initialise new Megamenu block @block_delta which already exists.', array('@block_delta' => $this->id())), 'error');
      return FALSE;
    }

    $this->configuration = $this->config_defaults($this->id());

    // Add the new megamenu to the list of loaded menus.
    greyhead_megamenu_menus_loaded($this);

    $this->save_pending(TRUE);
  }

  /**
   * Get the defaults for a new Megamenu.
   *
   * @return array
   */
  function config_defaults() {
    $defaults = array(
      // default is TRUE when the block is new and unconfigured.
      'default' => TRUE,
      'menu' => array(
        'name' => variable_get('menu_main_links_source', 'main-menu'),
      ),
      'megamenu' => array(
        //
        // The id must be globally unique and is the menu block's machine name.
        // It always starts with the parent menu's name, with hyphens converted
        // to underscores, e.g. if the menu name was "user-menu", and the
        // user creating the megamenu block chose the machine name "steve",
        // the id would be "user_menu__steve". The ID is the same as the block
        // delta passed to hook_block_view.
        //
        'id' => $this->id(),
        'classes' => array('greyhead-megamenu'),
        //
        // default item config contains the defaults applied to every new
        // top-level menu item when a new link is added.
        //
        'default-menuitem-config' => $this->menuitem_config_defaults(FALSE),
        //
        // menuitems contains configuration information for each individually-
        // configured top-level menu entry, keyed by the link's mlid for menu
        // links and the insert_id for inserts.
        //
        // When a new top-level menu link is detected, a copy of
        // default-item-config is created for it, and the block configuration
        // is updated.
        //
        // @see also greyhead_megamenu_menuitem_defaults()
        //
        'menuitems-config' => array(),
        //
        // inserts are additional items shown as if they were top-level menu
        // links, but they can contain static HTML or the contents of a block.
        //
        'inserts' => array(
          '#items' => array(
            // items is a list of inserts, where the array key is in the form
            // megamenu-insert-n where n is the insert's number, starting from
            // 0. @see also insert_defaults()
          ),
        ),
      ),
      // Settings for the Drupal block itself.
      'block' => array(
        'subject' => NULL,
      ),
    );

    // Implement hook_greyhead_megamenu_menu_config_defaults_alter.
    drupal_alter('greyhead_megamenu_menu_config_defaults', $defaults);

    return $defaults;
  }

  /**
   * Get the default options for a menuitem (a megamenu tab).
   *
   * These defaults can either come from customised options configured for the
   * current Megamenu or from code defaults, for example if the megamenu is new.
   *
   * @param bool|TRUE $load_from_configuration
   * @param null      $menuitem_id
   *
   * @return array
   */
  function menuitem_config_defaults($load_from_configuration = TRUE, $menuitem_id = NULL) {
    // If $load_from_configuration is true, this function has probably been
    // called from a function which is creating a menuitem. If FALSE, it has
    // probably been called from within config_defaults().
    if ($load_from_configuration) {
      $menuitem_config_defaults = $this->configuration['megamenu']['default-menuitem-config'];
      $menuitem_config_defaults['id'] = $menuitem_id;

      // Implement hook_greyhead_megamenu_menuitem_config_defaults_alter. This
      // hook provides the block_delta as context.
      $block_delta = $this->id();
      drupal_alter('greyhead_megamenu_menuitem_config_defaults', $menuitem_config_defaults, $block_delta);
    }
    else {
      $menuitem_config_defaults = array(
        // id is the mlid of the menu link, or the insert ID (e.g. insert1).
        'id' => $menuitem_id,
        // customised is set to FALSE when the menu item has never been edited,
        // and TRUE if it has.
        'customised' => FALSE,
        // type can be either 'menulink' or 'insert'.
        'type' => 'menulink',
        // Set this to false if you don't want to display a megamenu under this
        // menuitem.
        'show-megamenu' => TRUE,
        // columns contains configuration for the megamenu's columns.
        'columns' => array(
          'options' => array(
            'count' => 3,
          ),
          '#items' => array(
            0 => array(
              'width' => 3,
              'content' => array(
                // Type can be submenu, block or node.
                'type' => 'block',
                'options' => array(
                  'module' => 'greyhead_megamenu',
                  'delta' => 'sample_block',
                ),
              ),
            ),
            1 => array(
              'width' => 5,
              'content' => array(
                'type' => 'submenu',
                // Submenu blocks can be further subdivided into 1 - 6 columns.
                'options' => array(
                  'columns_count' => 2,
                ),
              ),
            ),
            2 => array(
              'width' => 4,
              'content' => array(
                // Type can be submenu, block or node.
                'type' => 'block',
                'options' => array(
                  'module' => 'greyhead_megamenu',
                  'delta' => 'sample_block',
                ),
              ),
            ),
          ),
        ),
      );

      // Implement hook_greyhead_megamenu_menuitem_config_defaults_alter.
      drupal_alter('greyhead_megamenu_menuitem_config_defaults', $menuitem_config_defaults);
    }

    return $menuitem_config_defaults;
  }

  /**
   * Write a Menublock's configuration to the database.
   *
   * This function is expensive, so you should instead call
   * $this->save_pending(TRUE). This will queue any changes to the Megamenu to
   * be saved on hook_exit().
   */
  function save() {
    $this->save_pending(FALSE);
    variable_set($this->get_config_variable_name(), $this->configuration);
  }

  /**
   * Creates a default configuration for a menu item.
   *
   * This is usually called when a new menu item is found or created.
   *
   * @param $menuitem_id
   *
   * @return array
   */
  function menuitem_config_create($menuitem_id) {
    return $this->menuitem_config_defaults(TRUE, $menuitem_id);
  }

  /**
   * Retrieves an insert from the Megamenu's configuration.
   *
   * @param $insert_id
   *
   * @return bool
   */
  function get_insert($insert_id) {
    $inserts = $this->get_inserts();

    if (isset($inserts[$insert_id])) {
      return $inserts[$insert_id];
    }
  }

  /**
   * Get or set configured inserts from the Megamenu's configuration.
   *
   * To set (i.e. update) the list of inserts, pass an array whose keys are
   * insert IDs (e.g. insert1, insert2 ... insertn) and whose values are valid
   * inserts, based on insert_defaults().
   *
   * @param null|array $inserts
   *
   * @return mixed
   */
  function get_inserts($inserts = NULL) {
    if (is_null($inserts)) {
      return $this->configuration['megamenu']['inserts']['#items'];
    }
    elseif (is_array($inserts)) {
      // Update the inserts.
      $this->configuration['megamenu']['inserts']['#items'] = $inserts;
      $this->save_pending(TRUE);
    }
  }

  /**
   * Save a new or existing insert to the Megamenu's configuration.
   *
   * @param $insert
   */
  function save_insert($insert) {
    // If $insert['id'] is NULL, we're creating a new insert.
    if (is_null($insert['id'])) {
      $insert['id'] = 'insert' . count($this->configuration['megamenu']['inserts']['#items']);
    }

    $this->configuration['megamenu']['inserts']['#items'][$insert['id']] = $insert;
    $this->save_pending(TRUE);
  }

  /**
   * Get the default options for a Megamenu insert.
   *
   * @return array
   */
  function insert_defaults() {
    $insert_defaults = array(
      // id is insert's identifier and is the same as the insert's key in the
      // inserts array. Example IDs: "insert1", "insert2" ... "insertn".
      'id' => NULL,
      // The weight is used to position this insert in the menu.
      'weight' => NULL,
      // The content array is used to built the insert's content
      'content' => array(
        // type can (currently) be only "static-html" or "block".
        'type' => 'static-html',
        'options' => array(
          'html' => 'This is some static HTML.',
        ),
      ),
    );

    // Implement hook_greyhead_megamenu_insert_defaults_alter.
    $megamenu_id = $this->id();
    drupal_alter('greyhead_megamenu_insert_defaults', $insert_defaults, $megamenu_id);

    return $insert_defaults;
  }

  /**
   * Gets the Megamenu menuitems for this menu.
   *
   * Each menuitem contains the item's configuration, the item itself - which
   * may be either a Drupal link or an insert - and any megamenu content.
   *
   * @return mixed
   */
//  function menuitems_config_get() {
//    $menuitems_config = &drupal_static(__FUNCTION__ . ':' . $this->id());
//
//    // @TODO: implement caching here.
//
//    if (is_null($menuitems_config)) {
//      $menuitems_config = array();
//
//      // Get the top-level menu items for this menu.
//      $this->menuitems_config_add_menulinks($menuitems_config);
//
//      // Get any inserts.
//      $this->menuitems_config_add_inserts($menuitems_config);
//
//      // Get the megamenus' content.
//      $this->menuitems_config_add_menulinks_content($menuitems_config);
//
//      // Sort the items.
//      ksort($menuitems_config);
//    }
//
//    // Return the items.
//    dpm($menuitems_config);
//    return $menuitems_config;
//  }

//  function menuitems_config_add_menulinks(&$menuitems) {
//    foreach (menu_tree_all_data($this->configuration['menu']['name'], NULL, 1) as $sortvalue_title_and_mlid => $menu_link_and_below) {
//      /**
//       * $sortvalue_title_and_mlid is 50000 plus the menu item's weight, the
//       * menu item's localised title, and the mlid - we use a similar technique
//       * with inserts to allow us to order them by Drupal weight.
//       *
//       * @see menu_tree_check_access
//       */
//      dpm($menu_link_and_below);
//      $menuitem_id = $menu_link_and_below['link']['mlid'];
//      $menuitem_link = $menu_link_and_below['link'];
//
//      $menuitem = $this->menuitem_config_build($menuitem_id, $menuitem_link);
//
//      $menuitems[$sortvalue_title_and_mlid] = $menuitem;
//    }
//  }

//  function menuitem_config_build($menuitem_id, $menuitem_item, $menuitem_config = NULL) {
//    $menuitem = array(
//      'id' => $menuitem_id,
//      'menuitem' => $menuitem_item,
//      'options' => !is_array($menuitem_config) ? array() : $menuitem_config,
//      'submenu' => NULL,
//    );
//
//    $this->check_for_new_menuitem($menuitem);
//
//    // Implement hook_greyhead_megamenu_menuitem_build_alter.
//    drupal_alter('greyhead_megamenu_menuitem_build', $menuitem);
//
//    return $menuitem;
//  }

//  function menuitems_config_add_inserts(&$menuitems) {
//    foreach ($this->get_inserts() as $menuitem_id => $insert) {
//      /**
//       * $sortvalue_title_and_mlid is 50000 plus the menu item's weight, the
//       * menu item's localised title, and the mlid - we use a similar technique
//       * with inserts to allow us to order them by Drupal weight.
//       *
//       * @see menu_tree_check_access
//       */
//      $menuitem_id_with_weight = (50000 + $insert['weight']) . ' ' . $menuitem_id;
//      $menuitem_link = $insert;
//
//      $menuitem = $this->menuitem_config_build($menuitem_id, $menuitem_link);
//
//      $menuitems[$menuitem_id_with_weight] = $menuitem;
//    }
//  }

  /**
   * Assemble a block containing the Megamenu as a renderable array.
   *
   * @return array
   */
  function build() {
    // Include JS and CSS.
    drupal_add_js(drupal_get_path('module', 'greyhead_megamenu') . '/js/greyhead-megamenu.js', 'file');
    drupal_add_css(drupal_get_path('module', 'greyhead_megamenu') . '/css/greyhead-megamenu.css');

    $block = array(
      // The block title.
      'subject' => $this->configuration['block']['subject'],
      // The block's content, as a renderableableable array.
      'content' => array(
        'element' => array(
          '#theme' => 'greyhead_megamenu_megamenu',
          '#id' => drupal_html_id('megamenu-' . $this->id()),
          '#attributes' => array(
            'class' => $this->configuration['megamenu']['classes'],
          ),
          '#menuitems' => $this->menuitems_output(),
        ),
      ),
    );

//    dpm($block);

    return $block;
  }

  /**
   * This function gets a list of the top-level menu items and any inserts.
   *
   * @return array An array of menu links, where the key is an arbitrary sort
   *               value, and the contents are a render array, which in turn
   *               contains render arrays for the menuitems' links, inserts and
   *               submenus.
   */
  function menuitems_output($reset = FALSE) {
    // Generate a cache ID (cid) specific for this $menu_name, $link, $language, and depth.
    global $user;
    $roles = $user->roles;

    // If the user is UID 1, add in "uid1" too, to distinguish from other
    // authenticated users.
    if ($user->uid == 1) {
      $roles[] = 'uid1';
    }

    sort($roles);
    $roles_string = implode('|', str_replace(' ', '-', $roles));

    $cid = substr('greyhead_megamenu:lang-' . $GLOBALS['language']->language . ':roles-' . $roles_string . ':id-' . $this->id() . ':menuitems', 0, 255);

    $menuitems = &drupal_static($cid);

    // If the static variable doesn't have the data, check {cache}.
    if (is_null($menuitems) || $reset
      || variable_get('greyhead_megamenu_cache_disabled')) {
      if (!variable_get('greyhead_megamenu_cache_disabled')) {
        $cache = cache_get($cid);
      }

      if (!$reset && !variable_get('greyhead_megamenu_cache_disabled')
        && $cache && is_array($cache->data)) {
        // If the cache entry exists, yay.
        $menuitems = $cache->data;
      }

      // If the menuitems for this megamenu weren't in the cache, build now.
      if (is_null($menuitems) || $reset
        || variable_get('greyhead_megamenu_cache_disabled')) {
        $menuitems = $this->build_menuitems();

        // Sort the items.
        ksort($menuitems);

        if (!variable_get('greyhead_megamenu_cache_disabled')) {
          // Store $menuitems in the cache.
          cache_set($cid, $menuitems, 'cache', (REQUEST_TIME + $this->cache_time()));
        }
      }
    }

//    dpm($menuitems);
//    dpm('$cid');
//    dpm($cid);

    // Return the items.
    return $menuitems;
  }

  /**
   * Assemble the menu items for a Megamenu.
   *
   * This function is expensive, so you should always use menuitems_output() if
   * possible.
   *
   * @return array
   */
  function build_menuitems() {
    $menuitems = array();

    // Get menu items.
    foreach (menu_tree_all_data($this->configuration['menu']['name'], NULL, 1) as $sortvalue_title_and_mlid => $menu_link_and_below) {
      /**
       * $sortvalue_title_and_mlid is 50000 plus the menu item's weight, the
       * menu item's localised title, and the mlid - we use a similar technique
       * with inserts to allow us to order them by Drupal weight.
       *
       * @see menu_tree_check_access
       */

      $menulink = $menu_link_and_below['link'];
      $menuitem_id = $menulink['mlid'];
      $menuitems[$sortvalue_title_and_mlid] = $this->output_menuitem($menuitem_id, $this->output_menulink($menulink), 'menulink');
    }

    // Get inserts - e.g. blocks or static HTML chunks to be added.
    foreach ($this->get_inserts() as $menuitem_id => $insert) {
      /**
       * $sortvalue_title_and_mlid is 50000 plus the menu item's weight, the
       * menu item's localised title, and the mlid - we use a similar technique
       * with inserts to allow us to order them by Drupal weight.
       *
       * @see menu_tree_check_access
       */

//      $menulink_render_array = array(
//        '#id' => $menuitem_id, // <-- Do we need this?
//        '#theme' => 'greyhead_megamenu_megamenu_item',
//        '#tabtype' => 'insert',
//        '#config' => $this->menuitem_config_get($menuitem_id),
//        '#menulink' => $this->insert_output($insert),
//        '#submenu' => $this->submenu_output($menuitem_id),
//      );

      $menuitem_id_with_weight = (50000 + $insert['weight']) . ' ' . $menuitem_id;
      $menuitems[$menuitem_id_with_weight] = $this->output_menuitem($menuitem_id, $this->output_insert($insert), 'insert');
    }

    // Implement a hook_greyhead_megamenu_menuitems_alter.
    $megamenu_id = $this->id();
    drupal_alter('greyhead_megamenu_menuitems', $menuitems, $megamenu_id);

    return $menuitems;
  }

  /**
   * Create a renderableable array for a menuitem.
   *
   * @param $menuitem_id
   * @param $thingbob
   * @param $thingbob_type
   *
   * @return array
   */
  function output_menuitem($menuitem_id, $thingbob, $thingbob_type) {
    $render_array = array(
      '#theme' => 'greyhead_megamenu_megamenu_item',
      '#id' => drupal_html_id('megamenu-' . $this->id() . '-menuitem-' . $menuitem_id),
      '#tabtype' => $thingbob_type,
      '#config' => $this->menuitem_config_get($menuitem_id),
      'menulink' => array(
        '#theme' => 'greyhead_megamenu_megamenu_item_' . $thingbob_type,
        // It's late and I'm tired, so 'thingbob' is a perfectly acceptaboo
        // name.
        '#item' => $thingbob,
      ),
      'submenu' => $this->submenu_output($menuitem_id),
    );

    return $render_array;
  }

  /**
   * Get the configuration for a menu item.
   *
   * @param           $menuitem_id
   * @param bool|TRUE $recurse
   *
   * @return mixed
   */
  function menuitem_config_get($menuitem_id, $recurse = TRUE) {
    if (isset($this->configuration['megamenu']['menuitems-config'][$menuitem_id])) {
//      dpm($this->configuration['megamenu']['menuitems-config'][$menuitem_id]);
      return $this->configuration['megamenu']['menuitems-config'][$menuitem_id];
    }
    else {
//      dpm($menuitem_id);
      $this->menuitem_config_set($this->menuitem_config_defaults(TRUE, $menuitem_id));

      // Don't allow infinite loops, e.g. if something goes askew during save.
      if ($recurse) {
        return $this->menuitem_config_get($menuitem_id, FALSE);
      }
    }
  }

  /**
   * Save the configuration for a menu item.
   *
   * @param $menuitem
   */
  function menuitem_config_set($menuitem) {
//    dpm($menuitem);
    if (isset($menuitem['id'])) {
      $this->configuration['megamenu']['menuitems-config'][$menuitem['id']] = $menuitem;
      $this->save_pending(TRUE);
    }
  }

//  function check_for_new_menuitem(&$menuitem) {
//    // Check if the item is new - if it is, it won't have any configuration
//    // yet so we need to create from defaults.
//    $menuitem['options'] = $this->menuitem_config_get($menuitem['id']);
//
//    if (!$menuitem['options']) {
//      $menuitem = $this->menuitem_config_create($menuitem['id']);
//      $this->menuitem_config_set($menuitem);
//    }
//  }

//  function menuitems_config_add_menulinks_content(&$menuitems) {
//    // Loop through each menuitem.
//    foreach ($menuitems as $menuitem_id => &$menuitem) {
//      // First, check if we're showing a megamenu at all.
//      if (!$menuitem['options']['show-megamenu']) {
//        continue;
//      }
//
//      // Now we need to go through each configured column and build its output.
//      foreach ($menuitem['options']['columns']['#items'] as $column_id => $column_configuration) {
//        $menuitem['submenu']['columns'][$column_id] = $this->build_column($column_configuration, $column_id, $menuitem);
//      }
//    }
//  }

  /**
   * Builds a renderable array for a submenu, if one is configured to be shown.
   *
   * @param $menuitem_id
   *
   * @return array|void
   */
  function submenu_output($menuitem_id) {
    $menuitem_config = $this->menuitem_config_get($menuitem_id);

    // First, check if we're showing a megamenu at all.
    if (!$menuitem_config['show-megamenu']) {
      return FALSE;
    }

    $render_array = array(
      '#theme' => 'greyhead_megamenu_submenu',
      '#id' => drupal_html_id('megamenu-' . $this->id() . '-menuitem-' . $menuitem_id . '-submenu'),
      '#columns' => $this->get_columns($menuitem_id),
    );

    return $render_array;
  }

  /**
   * Build the renderable arrays for columns for a menu item.
   *
   * @param $menuitem_id
   *
   * @return array
   */
  function get_columns($menuitem_id) {
    $menuitem_config = $this->menuitem_config_get($menuitem_id);
//    dpm($menuitem_config);

    $columns = array();

    foreach ($menuitem_config['columns']['#items'] as $column_id => $column_configuration) {
      $columns[$column_id] = $this->build_column($column_configuration, $column_id, $menuitem_config);
    }

    return $columns;
  }

  /**
   * Builds the renderaboo array for a column.
   *
   * @param $column_configuration
   * @param $column_id
   * @param $menuitem
   *
   * @return array
   */
  function build_column($column_configuration, $column_id, $menuitem) {
    // Create the column's render array.
    $render_array = array(
      '#theme' => 'greyhead_megamenu_submenu_column',
      '#id' => drupal_html_id('megamenu-' . $this->id() . '-menuitem-' . $menuitem['id'] . '-submenu-column-' . $column_id),
      '#column_class' => 'col-sm-' . $column_configuration['width'],
      'content' => NULL,
    );

    // Get the callback to build the column.
    $callback = $this->get_column_builder_callback($column_configuration['content']);

    if (function_exists($callback)) {
      $render_array['content'] = call_user_func($callback, $column_configuration['content']['options'], $column_id, $menuitem, $this);
    }

    return $render_array;
  }

  /**
   * Get the function name which will build a column.
   *
   * @param $column_content_options
   *
   * @return bool
   */
  function get_column_builder_callback($column_content_options) {
    $callback = FALSE;

    // Implement a hook_greyhead_megamenu_column_builder_callbacks_alter.
    // @see greyhead_megamenu.module >
    // greyhead_megamenu_greyhead_megamenu_column_builder_callbacks_alter
    // for the hook_alter which populates the default callbacks.
    $megamenu_id = $this->id();
    drupal_alter('greyhead_megamenu_column_builder_callbacks', $callback, $column_content_options, $megamenu_id);

    return $callback;
  }

  /**
   * Turns a standard Drupal menu link into a renderaboo array.
   *
   * Yes, I'm sure there's a function that already does this, or it's
   * unneccessary. Do you think I could find the answer? Of course not...
   *
   * @param $menulink
   *
   * @return array
   */
  function output_menulink($menulink) {
    $output = array(
      '#markup' => l($menulink['link_title'], $menulink['link_path'], $menulink['localized_options']),
    );

    // Implement hook_greyhead_megamenu_menulink_output_alter.
    // @TODO: should we allow hook functions to change the whole $output array?
    $megamenu_id = $this->id();
    drupal_alter('greyhead_megamenu_menulink_output', $output['#markup'], $menulink['mlid'], $menulink, $megamenu_id);

    return $output;
  }

  /**
   * Create a renderable array from anm insert.
   *
   * @param $insert
   *
   * @return array
   */
  function output_insert($insert) {
    $output = array();

    switch ($insert['content']['type']) {
      case 'static-html':
        $output['#markup'] = $insert['content']['options']['html'];
        break;

      case 'block':
        $block = module_invoke($insert['content']['options']['module'], 'block_view', $insert['content']['options']['delta']);
        $output = $block['content'];
        break;
    }

    // Implement hook_greyhead_megamenu_insert_output_alter.
    drupal_alter('greyhead_megamenu_insert_output', $output, $insert['content']['type'], $insert);

    return $output;
  }

  /**
   * Get or set the number of seconds that rendered megamenu blocks should be
   * cached.
   *
   * @param null $cache_time_seconds
   *
   * @return null
   */
  function cache_time($cache_time_seconds = NULL) {
    if (is_numeric($cache_time_seconds) && (intval($cache_time_seconds) == $cache_time_seconds)) {
      variable_set('greyhead_megamenu_cache_time', $cache_time_seconds);
    }
    else {
      return variable_get('greyhead_megamenu_cache_time', GREYHEAD_MEGAMENU_CACHE_TIME_DEFAULT);
    }
  }
}
