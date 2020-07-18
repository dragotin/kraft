## How to Contribute

There are two ways to contribute to the user manual of Kraft.

### Work on the English Manual

Just improve the English Manual that is stored in the file `kraft.adoc` in this file.
With that you can concentrate on writing and do not deal with scripting etc. Content rules!
Please send your changes either as pull request, or via email to the project maintainers. They will care for the rest.

Every change that is done to the master document in English language will be translated
to all the other internationalized versions of the doc.

### Translate

The translations of Kraft will be maintained on Transifex, just like the normal
software strings. Check there for translation tasks.

## Internationalization of the Manual

This is based on the great [doc here](https://github.com/KiCad/kicad-doc/blob/master/doc_alternatives/README.adoc).

The process of internationalization of the Kraft Manual is done in different steps.

It relies on the the same gettext process that is also used for the normal software
strings. To extract the strings from the manual, the great utility po4a is used.

### Step 1: String Template Extraction

This is extracting strings ready for the gettext system. This needs only to
be done once. Later, the pot file is only going to be updated.

```
  po4a-gettextize -f asciidoc -M utf-8 -m kraft.adoc -p po/kraft.pot
```

The update works with the command msgmerge:

```
  msgmerge -vU kraft-de.po kraft.pot
```

### Step 2: Translation

Copy the template into the nationalized version:

`cp po/kraft.pot po/kraft-de.po`

and use the gettext editor you like or upload to Transifex.

Keep in mind that snapshots images should be nationalized. I suggest to
create a internationalized image dirs such as:
```
  images
  images-de
  images-es
```
This way untranslated images fallback to English images. po4a
correctly translate image reference to enable the fallback.

### Step 3: Produce Internationalized Master Documents

```
  po4a-translate -f asciidoc -M utf-8 -m kraft.adoc -p po/kraft-de.po -k 0 -l kraft-de.adoc
```

### Step 4: Produce all Kind of Internationalized Output Formats

```
  asciidoc -a lang=de kraft-de.adoc    #convert into html
  a2x -a lang=de -f pdf kraft-de.adoc  #convert into pdf
  a2x -a lang=de -f epub kraft-de.adoc #convert into epub
```

### Step 5: Update Translations

With the following command the .po file will be updated automatically.

```
  po4a-updatepo -M utf8 -f asciidoc -m kraft.adoc -p po/kraft-de.po
```

### Step 6: Loop

repeat from step 2
