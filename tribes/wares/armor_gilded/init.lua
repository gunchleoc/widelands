dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "armor_gilded",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gilded Armor"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 1
   },
   preciousness = {
      empire = 1
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 4, 11 },
      },
   }
}
