dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "smoked_meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Smoked Meat"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20
   },
   preciousness = {
      atlanteans = 2
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 9, 16 },
      },
   }
}
