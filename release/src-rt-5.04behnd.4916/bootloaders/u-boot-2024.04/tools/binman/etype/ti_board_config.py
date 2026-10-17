# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2022-2023 Texas Instruments Incorporated - https://www.ti.com/
# Written by Neha Malcom Francis <n-francis@ti.com>
#
# Entry-type module for generating schema validated TI board
# configuration binary
#

import os
import struct
import yaml
import yamllint

from collections import OrderedDict
from jsonschema import validate
from shutil import copyfileobj

from binman.entry import Entry
from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools
from yamllint import config

BOARDCFG = 0xB
BOARDCFG_SEC = 0xD
BOARDCFG_PM = 0xE
BOARDCFG_RM = 0xC
BOARDCFG_NUM_ELEMS = 4

class Entry_ti_board_config(Entry_section):
    """An entry containing a TI schema validated board config binary

    This etype supports generation of two kinds of board configuration
    binaries: singular board config binary as well as combined board config
    binary.

    Properties / Entry arguments:
        - config-file: File containing board configuration data in YAML
        - schema-file: File containing board configuration YAML schema against
          which the config file is validated

    Output files:
        - board config binary: File containing board configuration binary

    These above parameters are used only when the generated binary is
    intended to be a single board configuration binary. Example::

        my-ti-board-config {
            ti-board-config {
                config = "board-config.yaml";
                schema = "schema.yaml";
            };
        };

    To generate a combined board configuration binary, we pack the
    needed individual binaries into a ti-board-config binary. In this case,
    the available supported subnode names are board-cfg, pm-cfg, sec-cfg and
    rm-cfg. The final binary is prepended with a header containing details about
    the included board config binaries. Example::

        my-combined-ti-board-config {
            ti-board-config {
                board-cfg {
                    config = "board-cfg.yaml";
                    schema = "schema.yaml";
                };
                sec-cfg {
                    config = "sec-cfg.yaml";
                    schema = "schema.yaml";
                };
            }
        }
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._config = None
        self._schema = None
        self._entries = OrderedDict()
        self._num_elems = BOARDCFG_NUM_ELEMS
        self._fmt = '<HHHBB'
        self._index = 0
        self._binary_offset = 0
        self._sw_rev = 1
        self._devgrp = 0

    def ReadNode(self):
        super().ReadNode()
        self._config = fdt_util.GetString(self._node, 'config')
        self._schema = fdt_util.GetString(self._node, 'schema')
        # Depending on whether config file is present in node, we determine
        # whether it is a combined board config binary or not
        if self._config is None:
            self.ReadEntries()
        else:
            self._config_file = tools.get_input_filename(self._config)
            self._schema_file = tools.get_input_filename(self._schema)

    def ReadEntries(self):
        """Read the subnodes to find out what should go in this image
        """
        for node in self._node.subnodes:
            if 'type' not in node.props:
                entry = Entry.Create(self, node, 'ti-board-config')
                entry.ReadNode()
                cfg_data = entry.BuildSectionData(True)
                entry._cfg_data = cfg_data
                self._entries[entry.name] = entry
        self._num_elems = len(self._node.subnodes)

    def _convert_to_byte_chunk(self, val, data_type):
        """Convert value into byte array

        Args:
            val: value to convert into byte array
            data_type: data type used in schema, supported data types are u8,
                u16 and u32

        Returns:
            array of bytes representing value
        """
        size = 0
        if (data_type == '#/definitions/u8'):
            size = 1
        elif (data_type == '#/definitions/u16'):
            size = 2
        else:
            size = 4
        if type(val) == int:
            br = val.to_bytes(size, byteorder='little')
        return br

    def _compile_yaml(self, schema_yaml, file_yaml):
        """Convert YAML file into byte array based on YAML schema

        Args:
            schema_yaml: file containing YAML schema
            file_yaml: file containing config to compile

        Returns:
            array of bytes repesenting YAML file against YAML schema
        """
        br = bytearray()
        for key, node in file_yaml.items():
            node_schema = schema_yaml['properties'][key]
            node_type = node_schema.get('type')
            if not 'type' in node_schema:
                br += self._convert_to_byte_chunk(node,
                                                node_schema.get('$ref'))
            elif node_type == 'object':
                br += self._compile_yaml(node_schema, node)
            elif node_type == 'array':
                for item in node:
                    if not isinstance(item, dict):
                        br += self._convert_to_byte_chunk(
                            item, schema_yaml['properties'][key]['items']['$ref'])
                    else:
                        br += self._compile_yaml(node_schema.get('items'), item)
        return br

    def _generate_binaries(self):
        """Generate config binary artifacts from the loaded YAML configuration file

        Returns:
            byte array containing config binary artifacts
            or None if generation fails
        """
        cfg_binary = bytearray()
        for key, node in self.file_yaml.items():
            node_schema = self.schema_yaml['properties'][key]
            br = self._compile_yaml(node_schema, node)
            cfg_binary += br
        return cfg_binary

    def _add_boardcfg(self, bcfgtype, bcfgdata):
        """Add board config to combined board config binary

        Args:
            bcfgtype (int): board config type
            bcfgdata (byte array): board config data
        """
        size = len(bcfgdata)
        desc = struct.pack(self._fmt, bcfgtype,
                            self._binary_offset, size, self._devgrp, 0)
        with open(self.descfile, 'ab+') as desc_fh:
            desc_fh.write(desc)
        with open(self.bcfgfile, 'ab+') as bcfg_fh:
            bcfg_fh.write(bcfgdata)
        self._binary_offset += size
        self._index += 1

    def _finalize(self):
        """Generate final combined board config binary

        Returns:
            byte array containing combined board config data
            or None if unable to generate
        """
        with open(self.descfile, 'rb') as desc_fh:
            with open(self.bcfgfile, 'rb') as bcfg_fh:
                with open(self.fh_file, 'ab+') as fh:
                    copyfileobj(desc_fh, fh)
                    copyfileobj(bcfg_fh, fh)
        data = tools.read_file(self.fh_file)
        return data

    def BuildSectionData(self, required):
        if self._config is None:
            self._binary_offset = 0
            uniq = self.GetUniqueName()
            self.fh_file = tools.get_output_filename('fh.%s' % uniq)
            self.descfile = tools.get_output_filename('desc.%s' % uniq)
            self.bcfgfile = tools.get_output_filename('bcfg.%s' % uniq)

            # when binman runs again make sure we start clean
            if os.path.exists(self.fh_file):
                os.remove(self.fh_file)
            if os.path.exists(self.descfile):
                os.remove(self.descfile)
            if os.path.exists(self.bcfgfile):
                os.remove(self.bcfgfile)

            with open(self.fh_file, 'wb') as f:
                t_bytes = f.write(struct.pack(
                    '<BB', self._num_elems, self._sw_rev))
            self._binary_offset += t_bytes
            self._binary_offset += self._num_elems * struct.calcsize(self._fmt)

            if 'board-cfg' in self._entries:
                self._add_boardcfg(BOARDCFG, self._entries['board-cfg']._cfg_data)

            if 'sec-cfg' in self._entries:
                self._add_boardcfg(BOARDCFG_SEC, self._entries['sec-cfg']._cfg_data)

            if 'pm-cfg' in self._entries:
                self._add_boardcfg(BOARDCFG_PM, self._entries['pm-cfg']._cfg_data)

            if 'rm-cfg' in self._entries:
                self._add_boardcfg(BOARDCFG_RM, self._entries['rm-cfg']._cfg_data)

            data = self._finalize()
            return data

        else:
            with open(self._config_file, 'r') as f:
                self.file_yaml = yaml.safe_load(f)
            with open(self._schema_file, 'r') as sch:
                self.schema_yaml = yaml.safe_load(sch)

            yaml_config = config.YamlLintConfig("extends: default")
            for p in yamllint.linter.run(open(self._config_file, "r"), yaml_config):
                self.Raise(f"Yamllint error: {p.line}: {p.rule}")
            try:
                validate(self.file_yaml, self.schema_yaml)
            except Exception as e:
                self.Raise(f"Schema validation error: {e}")

            data = self._generate_binaries()
            return data

    def SetImagePos(self, image_pos):
        Entry.SetImagePos(self, image_pos)

    def CheckEntries(self):
        Entry.CheckEntries(self)
