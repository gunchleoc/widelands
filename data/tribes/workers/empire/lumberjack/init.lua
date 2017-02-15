dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 6, 24 },
      fps = 10
   },
   hacking = {
      pictures = path.list_files(dirname .. "hacking_??.png"),
      hotspot = { 23, 23 },
      fps = 10
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 22}, 10)
add_walking_animations(animations, "walkload", dirname, "walkload", {9, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_lumberjack",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Lumberjack"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      felling_ax = 1
   },

   programs = {
      chop = {
         "findobject attrib:tree radius:10",
         "walk object",
         "play_sound sound/woodcutting fast_woodcutting 250",
         "animation hacking 10000",
--         "play_sound sound/spoken timber 156",
         "play_sound sound/woodcutting tree-falling 130",
         "object fall",
         "animation idle 2000",
         "createware log",
         "return"
      }
   },

   animations = animations,
}
