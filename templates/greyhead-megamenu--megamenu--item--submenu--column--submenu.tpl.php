<?php
/**
 * @file
 * greyhead-megamenu--megamenu--item--submenu--column--submenu.tpl.php
 *
 * This template outputs the submenu in one or more columns.
 *
 * This includes the second, third and fourth levels of links, where applicable.
 */
?>

<div class="container-fluid submenu">
  <?php foreach (element_children($element['#columns']) as $column_id): ?>
    <?php print render($element['#columns'][$column_id]) ?>
  <?php endforeach ?>
</div>
