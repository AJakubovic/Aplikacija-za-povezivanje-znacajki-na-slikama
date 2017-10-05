Aplikacija omogućava detekciju značajki na dvije slike neovisno, ekstrakciju značajki i kreiranje deskriptora, traženje 
podudarnosti među deskriptorima jedne i druge slike, te povezivanje parova značajki na osnovu toga. Zatim se vrši detekcija 
objekta sa prve slike na drugoj (npr. na prikazu koji uključuje i okolinu objekta – engl. scene). Za detekciju i ekstrakciju 
moguće je koristiti jedan od četiri algoritma: SIFT, SURF, BRISK ili ORB, a za povezivanje značajki Brute-Force matcher iz 
OpenCV biblioteke. Aplikacija pruža mogućnost nalaženja sličnosti između dvije slike, ukoliko sličnost postoji, ali i negiranja 
iste – u slučaju da nisu pronađene podudarne značajke na slikama.
