<?cs include:"templates/header.cs" ?>
<form method="post">
<?cs if:?error ?>
  <div class="error"><?cs var:error ?></div>
<?cs /if ?>
  <table class="user">
    <tr>
      <td><label for="login">Username</label></td>
      <td><input type="text" id="login" name="login" size="30" maxlength="30" class="focus"/></td>
    </tr>
    <tr>
      <td><label for="password">Password</label></td>
      <td><input type="password" id="password" name="password" size="30"/></td>
    </tr>
    <tr>
      <td/>
      <td class="right">
        <input type="submit" value="Login" name="submit"/>
      </td>
    </tr>
</table>
</form>
<?cs include:"templates/footer.cs" ?>
