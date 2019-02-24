<?cs include:"templates/header.cs" ?>
<div class="expand">
  <h1><?cs var:result ?></h1>
  <div class="controls">&nbsp;</div>
  <div class="expander">
  <hr/>
  <ul>
  <?cs each:item = log ?>
    <li><?cs var:item ?></li>
  <?cs /each ?>
  </ul>
  </div>
</div>
<?cs include:"templates/footer.cs" ?>
