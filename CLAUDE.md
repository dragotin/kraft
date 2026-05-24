# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What is Kraft

Kraft is a Qt6/KF6 Linux desktop application for small business document management — offers, invoices, delivery receipts. It generates PDF documents via WeasyPrint (HTML/CSS templates) and supports German e-invoicing (XRechnung). Data stays local: SQLite or MySQL, no cloud.

## Build

Out-of-source build in `build/`:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Run from the build directory with resources found in source:
```bash
export KRAFT_HOME=/path/to/kraft   # points to source root
./build/bin/kraft
```

Optional: disable Akonadi if not available:
```bash
cmake .. -DBUILD_WITH_AKONADI=OFF
```

Build the manual (requires `asciidoctor`):
```bash
make manual
```

## Tests

Tests use the Qt Test framework and link against `kraftlib` (the static lib). Run all tests:

```bash
cd build && ctest
```

Run a single test binary directly:
```bash
./build/tests/t_unitman
./build/tests/t_kraftdoc
```

Test sources are in `tests/`. Each `t_*.cpp` maps to one test binary.

## Code Formatting

clang-format is configured (`.clang-format`). Style is KDE/WebKit-based:
- 4-space indentation, 160-column limit
- `PointerAlignment: Right` (i.e., `QString *foo`, not `QString* foo`)
- Linux brace style (brace on new line for functions/classes)

## Architecture

### Core data model

- **`KraftObj`** (`src/kraftobj.h`) — base class for most domain objects. Provides UUID, last-modified timestamp, dirty flag, a typed attribute map (`KraftAttrib`), and a tag set. Serializes to/from XML via `kobjXml`/`parseKobjXml`.
- **`KraftDoc`** (`src/kraftdoc.h`) — the central document object, inherits `KraftObj`. Has a `KraftDocState` (Draft → Final → Retracted/Invalid). A document contains a list of `DocPosition` objects.
- **`DocPosition`** (`src/docposition.h`) — a line item, inherits `KraftObj`. Types: `Position`, `Text`, `ExtraDiscount`, `Demand`, `Alternative`. Each has quantity, unit, price, tax type.
- **`Geld`** (`src/geld.h`) — monetary value wrapper used throughout.
- **`dbID`** (`src/dbids.h`) — typed database row id.

### Persistence layer

Documents can be saved in two ways:
- **XML** (`DocumentSaverXML`, `src/documentsaverxml.h`) — current format. Each document is an XML file under `~/.local/share/kraft/xml/`. An index (`XmlDocIndex`, `src/xmldocindex.h`) maps UUIDs and idents to file paths and is backed by `~/.local/share/kraft/kraft.idx` (JSON).
- **Database** (`DocumentSaverDB`, `src/documentsaverdb.h`) — legacy format. Still used for catalog/template data.

`DefaultProvider` (`src/defaultprovider.h`) is a singleton that resolves paths for the Kraft v2 directory structure (`KraftV2Dir` enum: `XmlDocs`, `PdfDocs`, `NumberCycles`, etc.).

`KraftDB` (`src/kraftdb.h`) is a singleton wrapping Qt SQL. It handles schema version checks and migrations. Migration scripts live in `database/sqlite3/migration/` and `database/mysql/migration/`.

### Document generation (PDF)

`ReportGenerator` (`src/reportgenerator.h`) orchestrates PDF creation:
1. Picks a template file (`.gtmpl` for WeasyPrint, `.trml` for legacy ReportLab).
2. Renders via `GrantleeFileTemplate` (KTextTemplate) to produce an HTML intermediate.
3. Calls `PDFConverter` which shells out to `weasyprint`.
4. Optionally merges a PDF watermark via `pypdf2`.

Report templates live in `reports/`. `.gtmpl` files are Grantlee/KTextTemplate HTML templates; `.css` files control print layout. The portal HTML views in `views/` use the same templating system for on-screen display.

### Address handling

`AddressProvider` (`src/addressprovider.h`) abstracts over two backends:
- `AddressProviderAkonadi` — KDE Akonadi (optional, compile-time `#ifdef HAVE_AKONADI`)
- `AddressProviderLocal` — reads vCard files from a local contacts directory

### UI structure

- `Portal` (`src/portal.h`) — `QMainWindow`, the app's main window. Owns `PortalView` and manages open document editors.
- `PortalView` / `PortalHtmlView` — tabbed central widget; the "welcome" / document list area rendered as HTML.
- `KraftView` / `KraftViewBase` — the document editor. `KraftView` is editable; `KraftViewBase` also has `KraftView_ro` for read-only display.
- `AlldocsView` — the document list widget, backed by `DocumentModel` (a `QAbstractItemModel` in `src/models/`).

### Catalog / template system

Kraft has a catalog of reusable items (`TemplKatalog`, `MatKatalog`) and text fragments (`Floskel`). These are stored in the database and surfaced via catalog views. `TemplateProvider` and subclasses (header, footer, catalog) supply default texts for new documents.

### XRechnung (German e-invoicing)

`ExporterXRechnung` (`src/exportxrechnung.h`) generates a UN/CEFACT XML invoice from a `KraftDoc`. It uses the `.xrtmpl` template in `reports/`.

## Key conventions

- The app builds a static library `kraftlib` containing almost all source; the `kraft` executable just links it and provides `main.cpp`. Tests also link `kraftlib`.
- UI files (`.ui`) are processed by `ki18n_wrap_ui` (not plain `qt6_wrap_ui`) to support i18n. Generated headers land in the build directory.
- Settings are KConfig-based (`.kcfg` / `.kcfgc` files); generated classes `KraftSettings` and `DatabaseSettings` are in the build directory.
- `KRAFT_HOME` env var overrides the resource path at runtime (used for running from the build dir without installing).
