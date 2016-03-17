<?php
/**
 * @file
 * greyhead-megamenu--megamenu--item.tpl.php
 *
 * This template outputs the submenu container in a megamenu.
 */
?>

<li class="menu-item menu-item-type-<?php print $element['#tabtype'] ?>" id="<?php print $element['#id'] ?>">
  <div class="menu-item-inner">
    <?php print render($element['menulink']) ?>

    <?php print render($element['submenu']) ?>
  </div>
</li>
