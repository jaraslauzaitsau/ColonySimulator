# Roadmap

## Minimum Viable Product (MVP) description

![image](resources/RoadmapMVP.png)

### World generation

The world is made of islands and seas generated using Perlin noise.

### Colonizing feature

Every island has a cost of colonization that is paid in in-game resources: wood and iron. If a player has enough resources to colonize the island, then it will be added to the empire after clicking on the island. The price increases, as the islands get farther away from the main island and/or get bigger in area.

### Empire growth

Every set period of time (around a second), a growth tick happens, which increases the following resources:

- People. Growth depends on the amount of people already living in the empire. The formula looks something like that: (peopleAmount * marriedPercent) / 2, where marriedPercent is set to a balanced value
- Wood. Growth is calculated by the following formula: totalAreaColonized * woodGrowthFactor, where woodGrowthFactor is a balanced value

### Island resources

Every island has the following 2 resources in random quantities depending on the island's area: wood and iron. Wood quantities can increase (trees grow), while iron is not renewable. You can extract resources from the island and add them to your capital quicker by sending more people to islands.

### Indicators

Every island that hasn't been colonized yet has a lock icon in the middle of it with wood and iron quantities specified. If an island has already been colonized, then it shows the total wood and iron left with a (+x) attached at the end of the wood indicator, where x is the number of trees that grow per growth tick.

## Features planned

### v1.0.0 (MVP)

- [x] World generation
- [ ] Island extraction from the map
- [ ] Colonize the island by clicking it
- [ ] Empire's resources counters
- [ ] Draw a lock on top of uncolonized islands
- [ ] Set a price to colonize an island
- [ ] Colonized islands wood growth
- [ ] People on islands extract resources and add to the capital

### Additional features (DO NOT implement before MVP is reached)

- [ ] Ships flowing through the sea to the target island
- [ ] Animated people walking
