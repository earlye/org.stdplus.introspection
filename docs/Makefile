CHAPTERS=$(wildcard _chapters/chapter-*.markdown)

all : .build

publish : .build
	scp -r .build/* www.stdplus.org:~/subdomains/introspection

serve : .build
	bundle exec jekyll serve --destination .build --host 0.0.0.0 --port 4001

clean :
	-rm -rf .build

.PHONY: .build
.build : 
	jekyll build --destination $@
	rm $@/Makefile
	rm $@/Readme.md
