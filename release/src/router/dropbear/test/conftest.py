def pytest_addoption(parser):
    parser.addoption("--port", type=str, help="default is 2244 local, 22 remote")
    parser.addoption("--dbclient", type=str, default="../dbclient")
    parser.addoption("--dropbear", type=str, default="../dropbear")
    parser.addoption("--dropbearconvert", type=str, default="../dropbearconvert")
    parser.addoption("--dropbearkey", type=str, default="../dropbearkey")
    parser.addoption("--hostkey", type=str, help="required unless --remote")
    parser.addoption("--remote", type=str, help="remote host")
    parser.addoption("--user", type=str, help="optional username")
    parser.addoption("--ssh-keygen", type=str, default="ssh-keygen")

def pytest_configure(config):
    opt = config.option
    if not opt.hostkey and not opt.remote:
        raise Exception("--hostkey must be given")
    if not opt.port:
        if opt.remote:
            opt.port = "22"
        else:
            opt.port = "2244"

