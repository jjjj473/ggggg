# ggggg

This repository contains a prototype language called **UXC**. UXC aims to make
web UI markup and logic easier to write compared to raw HTML and JavaScript.

See `uxc/spec.md` for the language specification. The specification now
lists many proposed built-in systems. The translator implements about forty
of them, including attribute and style binding, two-way `:model` binding,
conditional and loop rendering, event modifiers and many additional helpers.

## Example

```
component HelloWorld
view:
  div id=main:
    h1: Hello UXC
    input :model=name persist=userName
    button onclick.prevent=sayHello :class=btnClass: Greet
    ul:
      li each=item in items :text=item
logic:
  var name = ''
  var btnClass = 'primary'
  var items = ['A','B','C']
  function sayHello() {
    alert('Hello ' + name);
  }
```

Use `translator.py` to compile a `.uxc` file to HTML:

```bash
python uxc/translator.py uxc/example.uxc > example.html
```

The resulting `example.html` contains standard HTML and JavaScript.
