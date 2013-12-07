from django.db             import models
from django.contrib.gis.db import models
from django.conf           import settings

class Tracker(models.Model):
	objects = models.GeoManager    (             )
	point   = models.PointField    ( srid = 4326 )
	date    = models.DateTimeField (             )
	def __unicode__(self):
		return u"(%s, %s)[%s]" % (self.position.x, self.position.y, date)