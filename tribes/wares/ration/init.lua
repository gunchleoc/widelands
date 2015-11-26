dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "ration",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Ration"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 20,
      empire = 20
   },
   preciousness = {
      barbarians = 5,
      empire = 4
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 3, 5 },
      },
   }
}
