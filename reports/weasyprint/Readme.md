## Weasyprint

This is a kind of case study if the [Weasyprint Project](https://weasyprint.org/)
gives a viable way to use it for Kraft and lets us go away from the cumbersome and
weakly maintained self knitted script based on Reportlab.

### Try it!

The example here can easily be built:

1. Install weasyprint preferably using packages from your distro, see this [Install Instructions](https://weasyprint.readthedocs.io/en/stable/install.html).
2. Go into the directory and call the command with input- and output file as parameters: `weasyprint invoice.html invoice.pdf`
3. Check the output file.

The appearance of the printed page is mostly influenced by the CSS (Cascading Style Sheet)
in file `invoice.css`.

