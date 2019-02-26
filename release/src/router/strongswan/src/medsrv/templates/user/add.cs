<?cs include:"templates/header.cs" ?>
<form method="post">
<?cs if:?error ?>
  <div class="error"><?cs var:error ?></div>
<?cs /if ?>
  <table class="user">
    <tr>
      <td><label for="new_login">Username</label></td>
      <td><input type="text" id="new_login" name="new_login" autofocus maxlength="30" value="<?cs var:new_login ?>"/></td>
    </tr>
    <tr>
      <td><label for="new_password">Password</label></td>
      <td><input type="password" id="new_password" name="new_password"/></td>
      <td><small>min. <?cs var:password_length ?> characters</small></td>
    </tr>
    <tr>
      <td><label for="confirm_password">Confirm password</label></td>
      <td><input type="password" id="confirm_password" name="confirm_password"/></td>
    </tr>
    <tr>
      <td/>
      <td class="right">
        <input type="submit" value="Register" name="register"/>
      </td>
    </tr>
</table>
</form>
<?cs include:"templates/footer.cs" ?>
