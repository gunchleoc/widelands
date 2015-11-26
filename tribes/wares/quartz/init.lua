dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "quartz",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Quartz"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 5
   },
   preciousness = {
      atlanteans = 1
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 7, 13 },
      },
   }
}
