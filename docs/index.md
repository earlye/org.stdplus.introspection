---
layout: page
---

# Overview

Introspection is a library for dealing with meta-information about your program.

This includes:

* Detecting compiler and platform features.
* Stack tracing
* Dynamic Library management

# Updates
{% for post in site.posts %}
* <span class="post-meta">{{ post.date | date: "%b %-d, %Y" }}</span> [{{ post.title | escape }}]({{ post.url | relative_url }})
{% endfor %}

[comment]: # It'd be cool to include repo logs here... https://help.github.com/articles/repository-metadata-on-github-pages/ 
