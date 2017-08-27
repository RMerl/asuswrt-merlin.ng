<?cs include:"templates/header.cs" ?>
<div>
<?cs if subcount(peers) > 0 ?>
<table class="list">
  <tr>
    <th>Alias</th>
    <th>Key Identifier</th>
  </tr>
<?cs each:peer = peers ?>
  <tr>
    <td>
      <a href="<?cs var:base ?>/peer/edit/<?cs name:peer?>"><?cs var:peer.alias ?></a>
    </td>
    <td>
      <a href="<?cs var:base ?>/peer/edit/<?cs name:peer?>"><?cs var:peer.identifier ?></a>
    </td>
  </tr>
<?cs /each ?>
</table>
<?cs else ?>
No peers defined.
<?cs /if ?>
<div class="right">
<form action="<?cs var:base ?>/peer/add" method="get">
    <input type="submit" value="Add Peer"/></td>
</form>
</div>
<?cs include:"templates/footer.cs" ?>
