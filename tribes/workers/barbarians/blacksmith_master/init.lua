dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 3, 22 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {6, 22}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {9, 23}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_blacksmith_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Master Blacksmith"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}
