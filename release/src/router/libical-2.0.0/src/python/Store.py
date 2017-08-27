#!/usr/bin/env python 
# -*- Mode: python -*-
#======================================================================
# FILE: Store.py
# CREATOR: eric 
#
# DESCRIPTION:
#   
#
#  $Id: Store.py,v 1.4 2002-07-08 17:56:11 acampi Exp $
#  $Locker:  $
#
# (C) COPYRIGHT 2001, Eric Busboom <eric@softwarestudio.org>
# (C) COPYRIGHT 2001, Patrick Lewis <plewis@inetarena.com>  
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of either: 
#
#    The LGPL as published by the Free Software Foundation, version
#    2.1, available at: http://www.fsf.org/copyleft/lesser.html
#
#  Or:
#
#    The Mozilla Public License Version 1.0. You may obtain a copy of
#    the License at http://www.mozilla.org/MPL/
#======================================================================

from LibicalWrap import *
from Error import LibicalError
from Component import Component, CloneComponent
from Gauge import Gauge

class Store:
    """ 
    Base class for several component storage methods 
    """

    class AddFailedError(LibicalError):
        "Failed to add a property to the file store"
        
    class ConstructorFailedError(LibicalError):
        "Failed to create a Store "
    
    def __init__(self):
        pass

    def path(self):
        pass
    
    def mark(self):
        pass
    
    def commit(self): 
        pass
    
    def add_component(self, comp):
        pass
    
    def remove_component(self, comp):
        pass
    
    def count_components(self, kind):
        pass
    
    def select(self, gauge):
        pass
    
    def clearSelect(self):
        pass
    
    def fetch(self, uid):
        pass
    
    def fetchMatch(self, comp):
        pass
    
    def modify(self, oldc, newc):
        pass
    
    def current_component(self):
        pass
    
    def first_component(self):
        pass
    
    def next_component(self):
        pass


class FileStore(Store):

    def __init__(self, file):
        e1=icalerror_supress("FILE")
        self._ref = icalfileset_new(file)
        icalerror_restore("FILE",e1)

        if self._ref == None or self._ref == 'NULL':
            raise  Store.ConstructorFailedError(file)

    def __del__(self):
        icalfileset_free(self._ref)

    def path(self):
        return icalfileset_path(self._ref)

    def mark(self):
        icalfileset_mark(self._ref)

    def commit(self): 
        icalfileset_commit(self._ref)

    def add_component(self, comp):
        if not isinstance(comp,Component):
            raise Store.AddFailedError("Argument is not a component")
            
        error = icalfileset_add_component(self._ref,comp.ref())

    def remove_component(self, comp):
        if not isinstance(comp,Component):
            raise Store.AddFailedError("Argument is not a component")

        error = icalfileset_remove_component(self._ref,comp.ref())

    def count_components(self, kind):
	_kind = icalcomponent_string_to_kind(kind)

	return icalfileset_count_components(self._ref, _kind)

    def select(self, gauge):
	error = icalfileset_select(self._ref, gauge.ref())

    def clearSelect(self):
	icalfileset_clear(self._ref)

    def fetch(self, uid):
	comp_ref = icalfileset_fetch(self._ref, uid)

        if comp_ref == None:
            return None

        return CloneComponent(comp_ref)

    def fetchMatch(self, comp):
        if not isinstance(comp,Component):
            raise Store.AddFailedError("Argument is not a component")
            
        comp_ref = icalfileset_fetch_match(self._ref,comp.ref())

        if comp_ref == None:
            return None

        return CloneComponent(comp_ref)

    def modify(self, oldc, newc):
	pass

    def current_component(self):
        comp_ref = icalfileset_get_current_component(self._ref)

        if comp_ref == None:
            return None

        return CloneComponent(comp_ref)

    def first_component(self):
        comp_ref = icalfileset_get_first_component(self._ref)

        if comp_ref == None:
            return None

        return CloneComponent(comp_ref)

    def next_component(self):
        
        comp_ref = icalfileset_get_next_component(self._ref)

        if comp_ref == None:
            return None

        return CloneComponent(comp_ref)

