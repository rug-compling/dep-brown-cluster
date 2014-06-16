# Dependency Brown clustering
## Syntactic extension of Brown et al. 1992 clustering algorithm


This is a modification of Percy Liang’s [implementation](http://github.com/percyliang/brown-cluster) (version 1.3) of the Brown hierarchical word clustering algorithm that is based on a **dependency language model** (DLM) instead of the bigram language model.

Note that this is not a revision of those segments of the original code which are not relevant for dependency clustering. The code modification should be seen as the minimal working extension of the original code for dependency-based clustering.

### Input

Tab-separated sequence of “head”, “dependent” and “count” (see [input.txt](input.txt) for an example), one such instance per line. Space-separated multiword sequences will be treated as one token. The program thus expects that the extraction of dependency instances with counts was already performed.

### Output

For each word type, its cluster (see [output.txt](output.txt) for an example). In particular, each line is: 

\[cluster bit id\] \[word\] \[number of times word occurs in input\]

### References

If you use this code, please cite:

-   Simon Šuster and Gertjan van Noord (2014) [From neighborhood to parenthood: the advantages of dependency representation over bigrams in Brown clustering.][] COLING. See also [induced clusters and experimental details](http://github.com/rug-compling/dep-brown-data).

Other references:

-   Brown, et al.: [Class-Based n-gram Models of Natural Language][]
-   Liang: [Semi-supervised learning for natural language processing][]
-   On dependency language models
    -   Chen et al. (2012) Utilizing Dependency Language Models for Graph-based Dependency Parsing Models
    -   Popel and Mareček (2010) Perplexity of n-Gram and Dependency Language Models
    -   Shen et al. (2008) A New String-to-Dependency Machine Translation Algorithm with a Target Dependency Language Model

### Compile

`make`

### Run

Cluster [input.txt](input.txt) into 50 clusters (–max-ind-level controls amount of verbose output):

    ./wcluster --text input.txt --c 50 --max-ind-level 3
    # Output in input-c50-p1.out/paths

### Changes for dependency clustering

Changes to the original code were made in the following files/functions:

-   wcluster.cc
    -   read\_text\_process\_word()
    -   read\_text()
    -   incorporate\_new\_phrase()
    -   create\_initial\_clusters()
    -   compute\_cluster\_distribs()
    -   main()
-   strdb.cc
    -   read\_text()

All modifications in the source code are marked as comments beginning with “dlm”.

  [From neighborhood to parenthood: the advantages of dependency representation over bigrams in Brown clustering.]: http://www.let.rug.nl/suster/publications/DepBrown.pdf
  [Class-Based n-gram Models of Natural Language]: http://dl.acm.org/citation.cfm?id-176316#
  [Semi-supervised learning for natural language processing]: http://cs.stanford.edu/~pliang/papers/meng-thesis.pdf

### Acknowledgments

Thanks to Percy Liang for clarifications about parts of original code.

### Copyright

(C) Copyright 2007-2012, Percy Liang 

(C) Copyright Simon Šuster

Permission is granted for anyone to copy, use, or modify these programs and accompanying documents for purposes of research or education, provided this copyright notice is retained, and note is made of any changes that have been made.

These programs and documents are distributed without any warranty, express or implied. As the programs were written for research purposes only, they have not been tested to the degree that would be advisable in any important application. All use of these programs is entirely at the user’s own risk.

<http://www.let.rug.nl/suster/>