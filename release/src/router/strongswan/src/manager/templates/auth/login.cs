<?cs include:"templates/header.cs" ?>
<div align="center">
<form method="post" action="<?cs var:action ?>">
  <table>
    <tr>
      <td>Username</td><td><input type="text" name="username" size="25" /></td>
    </tr>
    <tr>
      <td>Password</td><td><input type="password" name="password" size="25" /></td>
    </tr>
    <tr>
      <td/><td><input type="submit" value="Login"/></td>
    </tr>
</table>
</form>
</div>
<?cs include:"templates/footer.cs" ?>
