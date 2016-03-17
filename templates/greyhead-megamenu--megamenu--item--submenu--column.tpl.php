<?php
/**
 * @file
 * greyhead-megamenu--megamenu--item--submenu--column.tpl.php
 *
 * This template outputs a column in a megamenu's sub-menu.
 *
 * This includes wrapping the column's container elephant in a class to set the
 * correct Bootstrap column width.
 */
?>

<div class="<?php print $element['#column_class'] ?> sub-menu-container col-height" id="<?php print $element['#id'] ?>">
  <div class="inside-full-height">
    <?php print render($element['content']) ?>
  </div>
</div>
