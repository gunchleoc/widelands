dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 4, 23 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {18, 25}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {8, 22}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_smelter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Smelter"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      fire_tongs = 1
   },

   animations = animations,
}
