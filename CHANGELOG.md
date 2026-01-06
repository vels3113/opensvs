# Changelog

## 0.3 - 2026-01-06
- Enhanced massively JSON parser to support netgen JSON output (netgen 1.15.311 tested) 
- Added tree view for subcircuits and the corresponding subcircuit filter
- Added netgen command logging
- Various usability improvements

## 0.2 - 2025-12-03
- Reimplemented JSON parser to support netgen JSON output (netgen 1.15.311 tested). WARNING: netgen output support can be still limited
- Added Run -> LVS dock, it configures and shells out to `netgen -batch lvs ... -json`, writes `.out/.json` into the current directory, and loads the resulting JSON into the viewer
- UI tweaks: added borders to widget

## 0.1 - 2025-12-01
- Initial Qt6 GUI scaffold with summary + table view for netgen JSON reports
- JSON parser (for mock netgen output!) with fixture and QtTest coverage
- Filters: type combo + regex-capable search
- CLI support to load a JSON file at startup
- Welcome screen with load button
- Persistent recent files across sessions
- Status/log with timestamped entries, temp file logging, and dockable session log viewer
- About dialog with version and Qt info
