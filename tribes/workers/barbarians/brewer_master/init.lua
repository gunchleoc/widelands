dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle_\\d+.png" },
      hotspot = { 8, 26 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {15, 26}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {11, 24}, 10)


tribes:new_worker_type {
   name = "barbarians_brewer_master",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = _"Master Brewer",
   vision_range = 2,

	-- TRANSLATORS: Helptext for a worker: Master Brewer
   helptext = _"Produces the finest ales to keep warriors strong and happy in training.",
   animations = animations,
}
