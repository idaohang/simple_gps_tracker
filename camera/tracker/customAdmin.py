from django.forms             import *
from tajhiz.saisie.utils      import *
from tajhiz.saisie.models     import *
from django.contrib           import admin
from django.forms.widgets     import PasswordInput
from django.contrib.gis.admin import OSMGeoAdmin

class TrackerAdmin(OSMGeoAdmin):
    list_display        = ('id', 'date',)
    list_editable       = ('point' ,)
    list_display_links  = ('id',)
    search_fields       = ('id', 'date')
    list_per_page       = 10
    ordering            = ('date',)
    list_filter         = ('date',)
    save_as             = True
    list_select_related = True
    fieldsets = (
      ('Informations' ,       {'fields': ['id','date'], 'classes': ('show','extrapretty')}),
      ('Contour Geomertique', {'fields': ['point'    ], 'classes': ('show', 'wide'      )}),
    )
    scrollable = False
    map_width  = 400
    map_height = 325