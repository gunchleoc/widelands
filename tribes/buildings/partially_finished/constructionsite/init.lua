dirname = path.dirname(__file__)

tribes:new_constructionsite_type {
   msgctxt = "building",
   name = "constructionsite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings under construction
   descname = pgettext("building", "Construction Site"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = {
      -- The constructionsite is a mess. Not nice and clean, but rather some
      -- logs lying around on piles, maybe some tools.
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 5, 5 },
      },
      idle_with_worker = {
         pictures = path.list_files(dirname .. "idle_with_worker_??.png"),
         hotspot = { 33, 36 },
      }
   },

   aihints = {},
}
