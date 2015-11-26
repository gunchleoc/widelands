dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 3, 24 }
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 25}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {6, 23}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_miner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Miner"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      pick = 1
   },

   experience = 19,
   becomes = "barbarians_miner_chief",

   animations = animations,
}
