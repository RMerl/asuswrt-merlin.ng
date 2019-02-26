<?cs include:"templates/header.cs" ?>
<div class="dialog">
<form method="post" action="<?cs var:action ?>">
  <p>
    <select name="gateway" size="1">
      <?cs each:gateway = gateways ?>
        <option value="<?cs name:gateway ?>"><?cs var:gateway.name ?> (<?cs var:gateway.address ?>)</option>
      <?cs /each ?>
    </select>
  </p>
    <input type="submit" value="Select"/>
  <p>
</form>
</div>
<?cs include:"templates/footer.cs" ?>
