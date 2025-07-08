# ggggg

This repository contains a prototype language called **UXC**. UXC aims to make
web UI markup and logic easier to write compared to raw HTML and JavaScript.

See `uxc/spec.md` for the language specification. The specification now
includes a list of proposed built-in systems and logic features. The
translator implements a few of these systems, such as simple attribute binding
and conditional rendering.

## Example

```
component HelloWorld
view:
  div id=main:
    h1: Hello UXC
    button :title=btnTitle if=showButton onclick=sayHello: Click me
logic:
  var btnTitle = 'Click for greeting'
  var showButton = true
  function sayHello() {
    alert('Hello from UXC!');
  }
```

Use `translator.py` to compile a `.uxc` file to HTML:

```bash
python uxc/translator.py uxc/example.uxc > example.html
```

The resulting `example.html` contains standard HTML and JavaScript.
