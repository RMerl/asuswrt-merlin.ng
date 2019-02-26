<?cs include:"templates/header.cs" ?>
<?cs each:peercfg = peercfgs ?>
  <div class="expand" id="peercfg-<?cs name:peercfg ?>">
    <h1><?cs name:peercfg ?>:
  	  <span><?cs var:peercfg.local ?></span> &lt;-&gt; 
  	  <span><?cs var:peercfg.remote ?></span>
    </h1>
    <div class="controls">
      <?cs if:peercfg.remote != "%any" ?>
        <a title="initiate SA" href="<?cs var:base ?>/control/initiateike/<?cs name:peercfg ?>">
          <img src="<?cs var:base ?>/static/initiate.png"/>
        </a>
	  <?cs else ?>
        &nbsp;    
      <?cs /if ?>
    </div>
    <div class="expander">
      <hr/>
      <table class="drawing">
        <tr>
          <td class="left" colspan="3">
            <?cs var:peercfg.local ?>
          </td>
          <td>
          </td>
          <td class="right" colspan="3">
            <?cs var:peercfg.remote ?>
          </td>
        </tr>
        <tr class="images">
          <td>
            <?cs if:peercfg.remote != "%any" ?>
            <img title="Local host can be the initiator" src="<?cs var:base ?>/static/client-left.png"></img>
            <?cs else ?>
            <img title="Local host must be the responder" src="<?cs var:base ?>/static/gateway-left.png"></img>
            <?cs /if ?>
          </td>
          <td style="background-image:url(<?cs var:base ?>/static/pipe.png)">
            <font color="#e5bf5e">0123456789abdcef</font><br/><br/><br/>
            <?cs var:peercfg.ikecfg.local ?>
          </td>
          <td>
            <img src="<?cs var:base ?>/static/pipe.png"></img>
          </td>
          <td>
            <img title="IKE connection is down" src="<?cs var:base ?>/static/pipe.png"></img>
          </td>
          <td>
            <img src="<?cs var:base ?>/static/pipe.png"></img>
          </td>
          <td class="right" style="background-image:url(<?cs var:base ?>/static/pipe.png)">
            <font color="#e5bf5e">0123456789abcdef</font><br/><br/><br/>
            <?cs var:peercfg.ikecfg.remote ?>
          </td>
          <td>
            <?cs if:peercfg.remote == "%any" ?>
            <img title="Remote host must be the initiator" src="<?cs var:base ?>/static/client-right.png"></img>
            <?cs else ?>
            <img title="Remote host can be the responder" src="<?cs var:base ?>/static/gateway-right.png"></img>
            <?cs /if ?>
          </td>
        </tr>
        <?cs each:childcfg = peercfg.childcfgs ?>
        <tr>
          <td colspan="6" class="expand">
            <h1><?cs name:childcfg ?>:</h1>
          </td>
          <td class="controls">
            <?cs if:peercfg.remote != "%any" ?>
              <a title="initiate SA" href="<?cs var:base ?>/control/initiatechild/<?cs name:childcfg ?>">
                <img src="<?cs var:base ?>/static/initiate.png"/>
              </a>
            <?cs /if ?>
          </td>
        </tr>
        <tr>
          <td colspan="7"><hr/></td>
        </tr>
        <tr class="images">
          <td colspan="2">
            <?cs each:net = childcfg.local.networks ?>
      	      <p><?cs var:net ?></p>
            <?cs /each ?>
      	  </td>
          <td style="background-image:url(<?cs var:base ?>/static/pipe-thin-left.png)">
            <br/><br/><br/>
          </td>
          <td style="background-image:url(<?cs var:base ?>/static/pipe-thin.png)">
          </td>
          <td class="right" style="background-image:url(<?cs var:base ?>/static/pipe-thin-right.png)">
            <br/><br/><br/>
          </td>
          <td class="right" colspan="2">
            <?cs each:net = childcfg.remote.networks ?>
              <p><?cs var:net ?></p>
            <?cs /each ?>
          </td>
        </tr>
        <?cs /each ?>
      </table>
    </div>
  </div>
<?cs /each ?>
<?cs include:"templates/footer.cs" ?>
