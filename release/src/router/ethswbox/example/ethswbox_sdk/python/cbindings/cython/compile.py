from setuptools import setup
from setuptools import Extension
from Cython.Build import cythonize


ext_modules = [
    Extension(
        name="lif",
        sources=[
            "lif.pyx",
            "../../../src/lif/lif_api.c",
            "../../../src/lif/mdio/lib/mdio.c",
            "../../../src/lif/mdio/lib/bcm2835.c",
        ],
    )
]

setup(
    name="lif",
    include_dirs=[
        "../../../src",
        "../../../src/os",
        "../../../src/lif",
        "../../../src/lif/mdio/lib",
    ],
    ext_modules=cythonize(ext_modules),
)

# python3 compile.py build_ext --inplace
