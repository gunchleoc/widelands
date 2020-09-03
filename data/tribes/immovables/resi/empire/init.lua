push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_immovable_type {
   name = "empire_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/none_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "none",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Water"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/water_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "water",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Coal"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/coal_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "coal_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Gold"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/gold_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "gold_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Iron"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/iron_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "iron_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Marble"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/stone_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "stone_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Coal"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/coal_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "coal_much",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Gold"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/gold_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "gold_much",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Iron"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/iron_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "iron_much",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   name = "empire_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Marble"),
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/stone_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "stone_much",
         hotspot = {9, 19}
      }
   }
}

pop_textdomain()
