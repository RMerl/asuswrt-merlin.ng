<?cs include:"templates/header.cs" ?>
<form method="post">
<?cs if:?error ?>
  <div class="error"><?cs var:error ?></div>
<?cs /if ?>
  <table class="user">
    <tr>
      <td><label for="old_login">Username</label></td>
      <td><input type="text" id="old_login" name="old_login" maxlength="30" value="<?cs var:old_login ?>" /></td>
    </tr>
    <tr>
      <td><label for="old_password">Old password</label></td>
      <td><input type="password" id="old_password" name="old_password"/></td>
      <td></td>
    </tr>
    <tr>
      <td><label for="new_password">New password</label></td>
      <td><input type="password" id="new_password" name="new_password"/></td>
      <td><small>min. <?cs var:password_length ?> characters</small></td>
    </tr>
    <tr>
      <td><label for="confirm_password">Confirm new password</label></td>
      <td><input type="password" id="confirm_password" name="confirm_password"/></td>
    </tr>
    <tr>
      <td></td>
      <td class="right">
        <input type="submit" value="Back" name="back"/> 
        <input type="submit" value="Delete" name="delete" onclick="return confirm('Permanently delete your account?')"/> 
        <input type="submit" value="Save" name="save"/>
      </td>
    </tr>
  </table>
</form>
<?cs include:"templates/footer.cs" ?>
