push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "spider_silk",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Spider Silk"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 16 },
      },
   }
}

pop_textdomain()
