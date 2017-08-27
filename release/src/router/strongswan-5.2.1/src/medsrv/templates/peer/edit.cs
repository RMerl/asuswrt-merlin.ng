<?cs include:"templates/header.cs" ?>
<form method="post" name="form">
<?cs if:?error ?>
  <div class="error"><?cs var:error ?></div>
<?cs /if ?>
  <table class="peer">
    <tr>
      <td><label for="alias">Alias</label></td>
      <td><input type="text" id="alias" name="alias" maxlength="30" class="focus" value="<?cs var:alias ?>"/></td>
    </tr>
    <tr>
      <td valign="top"><label for="public_key">Public Key</label></td>
      <td><textarea id="public_key" name="public_key"><?cs var:public_key ?></textarea></td>
    </tr>
    <tr>
      <td></td>
      <td align="right">
        <input type="submit" value="Back" name="back"/>
        <input type="submit" value="Delete" name="delete" onclick="return confirm('Permanently delete this peer?')"/> 
      	<input type="submit" value="Save" name="save"/>
      </td>
    </tr>
</table>
</form>
<?cs include:"templates/footer.cs" ?>
