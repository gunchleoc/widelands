dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "waiting_???.png"),
      hotspot = { 11, 22 },
      fps = 10
   },
   work = {
      pictures = path.list_files(dirname .. "work_??.png"),
      sound_effect = {
            directory = "sound/hammering",
            name = "hammering",
      },
      hotspot = { 10, 22 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {7, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {7, 22}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Builder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hammer = 1
   },

   animations = animations,
}
