# -*- coding: utf-8 -*-
import textcat, unittest

class TestTextCat(unittest.TestCase):
    def setUp(self):
        self.classifier = textcat.TextCat("/usr/share/libtextcat/fpdb.conf", "/usr/share/libtextcat")

    def check_result(self, text, languages):
        # self.classifier.classify returns a list of the languages
        # it believes the input to be.
        self.assertEqual(self.classifier.classify(text), languages)

    def test_classify_english(self):
        self.check_result("""
        This text is supposed to be identified as english
        """, ['en--utf8'])

    def test_too_short(self):
        self.assertRaises(textcat.ShortException,
                self.classifier.classify, 'Short')

    def test_unknown(self):
        self.assertRaises(textcat.UnknownException,
                self.classifier.classify,
        """asdfasdfsafdasdf8uasdf89as7dfasfja89sdf7as89df7as9
        8df7ausf98masdfasdfafasklfjxajamsdfj""")

    def test_classify_norwegian_danish(self):
        # Too short to be determine just one.
        self.check_result("""
        Dette er ganske så norsk, men også litt dansk.
        """, ['nb--utf8', 'da--utf8'])

    def test_classify_norwegian(self):
        self.check_result(""" Tradisjonelt har en annen
        klassifisering enn den nevnt ovenfor vært brukt. Norsk har vært
        klassifisert som et vestnordisk språk sammen med islandsk og færøysk,
        mens dansk og svensk har vært klassifisert som østnordiske språk. Dette
        er basert på fonologiske forskjeller som oppsto tidlig i norrøn tid.
        Denne klassifiseringen er problematisk fordi den tillegger
        dialektforskjeller innenfor det fastlandsnordiske dialektkontinuumet
        større vekt enn skillet mellom tydelig adskilte språk bare fordi de
        førstnevnte forskjellene er eldre enn de sistnevnte. Dessuten går
        noen av skillene mellom vest- og østskandinavisk tvers igjennom det
        norske språkområdet, og videre har bokmål mange østnordiske trekk på
        grunn av sitt nære slektskap med dansk.
        """, ['nb--utf8'])

    def test_classify_german(self):
        self.check_result(""" Die deutsche Sprache (deutsch)
        gehört zum westlichen Zweig der germanischen Sprachen. Damit ist sie
        unter anderem mit dem Niederländischen und Englischen verwandt. Als
        Muttersprache hat Deutsch die meisten Sprecher in der Europäischen
        Union (EU).  Neben der Standardsprache besteht die deutsche Sprache in
        erster Linie aus einem Kontinuum verwandter Lokalmundarten, die sich in
        hochdeutsche und niederdeutsche Mundarten aufteilen lassen. Diese
        werden größtenteils von der deutschen Standardsprache überdacht, die
        auf der Grundlage hochdeutscher Mundarten entstanden ist und neben
        einfach Deutsch auch als Hochdeutsch und Standarddeutsch, als
        Schriftdeutsch oder als deutsche Literatursprache bezeichnet wird.
        """, ['de--utf8'])

if __name__ == '__main__':
        unittest.main()
