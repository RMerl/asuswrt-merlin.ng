from conf import hook

""" Test Option: EnvironmentVariables
This hook is used to define environment variables used for execution of wget
command in test."""


@hook(alias='EnvironmentVariables')
class URLs:
    def __init__(self, envs):
        self.envs = envs

    def __call__(self, test_obj):
        test_obj.envs.update(**self.envs)
