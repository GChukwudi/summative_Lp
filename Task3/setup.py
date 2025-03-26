from setuptools import setup, Extension

# Define the extension module
statistical_ext = Extension(
    'stat_extention',
    sources=['statistical_ext.c'],
    extra_compile_args=['-std=c99'],
    libraries=['m']
)

# Setup configuration
setup(
    name='stat_extention',
    version='1.0',
    description='Statistical Array Operations Extension',
    ext_modules=[statistical_ext]
)