using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Serialization;

namespace pvpgn_localize_validator
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                Console.WriteLine("This utility checks XML translation file for missing translations, arguments and illegal characters\n(c) 2014 HarpyWar (harpywar@gmail.com)");
                Console.WriteLine("\nUsage: {0} [filename.xml]\n", AppDomain.CurrentDomain.FriendlyName);

                Environment.Exit(0);
            }

            process_file(args[0]);
        }

        static void process_file(string filename)
        {
            try
            {
                XDocument doc = XDocument.Load(filename, LoadOptions.SetLineInfo);

                // * no author name
                var authors = doc.Descendants("author");
                foreach (var author in authors)
                {
                    if (author.Attribute("email").Value == "nomad@example.com")
                        logerror(author, "Please, remove or update example author.");
                }

                var items = doc.Descendants("item");
                XElement original, translate;
                XAttribute id;
                foreach (var item in items)
                {
                    original = item.Element("original");
                    translate = item.Element("translate");

                    // * check missing references
                    if ((id = translate.Attribute("refid")) != null)
                    {
                        if (items.Where(x => x.Attribute("id").Value == id.Value).Count() == 0)
                            logerror(translate, string.Format("string id=\"{0}\" is not found", id.Value));
                    }
                    else
                        check_string(original, translate);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("[ERROR] " + e.Message);
            }
        }


        static void check_string(XElement original, XElement translate)
        {
            string a = original.Value;
            string b = translate.Value;

            if (a.Trim().Length == 0)
            {
                logerror(translate, "empty original string");
                return;
            }
            if (b.Trim().Length == 0)
            {
                logerror(translate, "empty translation");
                return;
            }

            // * illegal characters
            if (!validate_symbols(a) || !validate_symbols(b))
                logerror(translate, string.Format("string contains one of invalid characters: {0}", string.Join(", ", invalid_symbols)));

            // * missing arguments
            if (a.Count(f => f == '{') != b.Count(f => f == '{') && a.Count(f => f == '}') != b.Count(f => f == '}'))
                logerror(translate, "missing argument in translation?");

            // * reduntant or missing spaces at start
            if (get_pos(a, ' ') != get_pos(b, ' '))
                logerror(translate, "spaces count at the beginning of the translation is not equal with original");

            // * reduntant or missing spaces at the end
            if (get_pos(a, ' ', true) != get_pos(b, ' ', true))
                logerror(translate, "spaces count at the end of the translation is not equal with original");
        }

        static int get_pos(string str, char c, bool reverse = false)
        {
            int pos = 0;
            if (!reverse)
                for (int i = 0; i < str.Length; i++)
                {
                    if (str[i] == c)
                        pos++;
                    else
                        break;
                }
            else
                for (int i = str.Length-1; i > 0; i--)
                {
                    if (str[i] == c)
                        pos++;
                    else
                        break;
                }
            return pos;
        }

        static string[] invalid_symbols = new string[]
        {
            ">",
            "<"
        };
        static bool validate_symbols(string str)
        {
            foreach(var s in invalid_symbols)
                if (str.Contains(s))
                    return false;
            return true;
        }

        static void logerror(XElement el, string text)
        {
            IXmlLineInfo info = el;
            int lineNumber = info.LineNumber;

            Console.WriteLine("[WARNING] line {0}: {1}", lineNumber, text);
        }
    }


}
