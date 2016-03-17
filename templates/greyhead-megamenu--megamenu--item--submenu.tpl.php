<?php
/**
 * @file
 * greyhead-megamenu--megamenu--item--submenu.tpl.php
 *
 * This template outputs the container for a megamenu's sub-menu.
 */
?>

<div class="megamenu-container" id="<?php print $element['#id'] ?>">
  <div class="container-fluid row clearfix">
    <div class="row-height">
      <?php foreach (element_children($element['#columns']) as $column_id): ?>
        <?php print render($element['#columns'][$column_id]) ?>
      <?php endforeach ?>
    </div>
  </div>
</div>
