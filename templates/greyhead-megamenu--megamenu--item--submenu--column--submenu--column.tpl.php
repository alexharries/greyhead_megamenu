<?php
/**
 * @file
 * greyhead-megamenu--megamenu--item--submenu--column--submenu--column.tpl.php
 *
 * This template outputs column within a submenu within a column. Confused? Yup.
 */
?>

<div class="submenu-column <?php print $element['#column_class'] ?>">
  <?php foreach (element_children($element['#items']) as $item_id): ?>
    <div class="submenu-column-menuitem">
      <h3 class="menu-link"><?php print l($element['#items'][$item_id]['#title'], $element['#items'][$item_id]['#href'], $element['#items'][$item_id]['#localized_options']) ?></h3>

      <?php if ($submenu_children = render($element['#items'][$item_id]['#below'])): ?>
        <?php print $submenu_children ?>
      <?php endif ?>
    </div>
  <?php endforeach ?>
</div>
