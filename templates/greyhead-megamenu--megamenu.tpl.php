<?php
/**
 * @file
 * greyhead-megamenu--megamenu.tpl.php
 *
 * This template outputs the megamenu's container and the already-rendered
 * megamenu content.
 *
 * This includes the first level of links, and any already-generated megamenus
 * belonging to those links.
 *
 * The megamenu is structured as follows:
 *
 *  - Megamenu
 *    - Item 1
 *      - Top-level link
 *      - Megamenu tab
 *        - Column 1 container
 *          - Node template (for example)
 *        - Column 2 container
 *          - Sub-menu template (for example)
 *        - Column 3 container
 *          - Block template (for example)
 *    - Item 2
 *      - Top-level link
 *      - Megamenu tab
 *        - Column 1 container
 *          - Node template (for example)
 *        - Column 2 container
 *          - Sub-menu template (for example)
 *        - Column 3 container
 *          - Block template (for example)
 *    - Item 3
 *      - Top-level link
 *      - Megamenu tab etc...
 */
?>

<div class="greyhead-megamenu-container">
  <div<?php print drupal_attributes($element['#attributes']) ?> id="<?php print $element['#id'] ?>">
    <ul class="menu">
      <?php foreach ($element['#menuitems'] as $menuitem): ?>
        <?php print render($menuitem) ?>
      <?php endforeach ?>
    </ul>
  </div>
</div>
