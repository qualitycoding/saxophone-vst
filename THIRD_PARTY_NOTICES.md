# Third-party notices

| Component | Licence | Used for | Shipped in binaries |
|---|---|---|---|
| [JUCE 9.0.3](https://github.com/juce-framework/JUCE) | AGPLv3 or commercial JUCE licence | Plugin framework (VST3 / AU / Standalone), GUI | Yes (statically linked). Distributing a binary requires complying with AGPLv3 or holding a JUCE licence. |
| VST 3 SDK (bundled with JUCE) | MIT (since VST 3.8) | VST3 wrapper | Yes |
| [nlohmann/json 3.12.0](https://github.com/nlohmann/json) | MIT | Parsing the resonator table and plugin state | Yes (header-only) |
| [Catch2 3.16.0](https://github.com/catchorg/Catch2) | BSL-1.0 | Test framework | No (tests only) |
| [pluginval 1.0.4](https://github.com/Tracktion/pluginval) | GPLv3 | Plugin validation in CI | No (downloaded in CI only) |
| [TinySOL](https://zenodo.org/record/3685367) (Cella et al., IRCAM) | CC BY 4.0 | Reference alto-saxophone recordings for the offline realism test | No (never shipped) |
| Colinot, Vergez, Guillemain & Doc (2021), Acta Acustica 5:33 | CC BY 4.0 | Model equations and Table 1/2 parameters of the saxophone model | Parameters/equations only |
| Szwarcberg, Colinot, Vergez & Jousserand (2025), Acta Acustica 9:57 | CC BY 4.0 | Simplified saxophone geometry used to generate the resonator table | Derived data only |

The source code of this repository is Apache-2.0 (see `LICENSE`, `NOTICE`).
