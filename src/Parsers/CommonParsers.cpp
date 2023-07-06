#include <Parsers/CommonParsers.h>
#include <base/find_symbols.h>
#include <algorithm>

namespace DB
{


namespace
{
    using Strings = std::vector<String>;

    class KeyWordToStringConverter
    {
    public:
        static const KeyWordToStringConverter & instance()
        {
            static const KeyWordToStringConverter res;
            return res;
        }

        const String & convert(Keyword type) const
        {
            return mapping[static_cast<size_t>(type)];
        }

        const std::vector<String> & getMapping() const
        {
            return mapping;
        }

    private:
        KeyWordToStringConverter()
        {
#define KEYWORD_TYPE_TO_STRING_CONVERTER_ADD_TO_MAPPING(identifier, value) \
            addToMapping(Keyword::identifier, value);

            APPLY_FOR_PARSER_KEYWORDS(KEYWORD_TYPE_TO_STRING_CONVERTER_ADD_TO_MAPPING)

#undef KEYWORD_TYPE_TO_STRING_CONVERTER_ADD_TO_MAPPING
        }

        void addToMapping(Keyword identifier, std::string_view value)
        {
            size_t index = static_cast<size_t>(identifier);
            mapping.resize(std::max(index + 1, mapping.size()));
            mapping[index] = value;
        }

        Strings mapping;
    };
}


const String & toStringRef(Keyword type)
{
    return KeyWordToStringConverter::instance().convert(type);
}

const std::vector<String> & getAllKeywords()
{
    return KeyWordToStringConverter::instance().getMapping();
}

ParserKeyword::ParserKeyword(Keyword keyword)
    : s(toStringRef(keyword))
{}

std::shared_ptr<ParserKeyword> ParserKeyword::createDeprecatedPtr(std::string_view s_)
{
    return std::shared_ptr<ParserKeyword>(new ParserKeyword(s_));
}

bool ParserKeyword::parseImpl(Pos & pos, [[maybe_unused]] ASTPtr & node, Expected & expected)
{
    if (pos->type != TokenType::BareWord)
        return false;

    const char * current_word = s.begin();

    while (true)
    {
        expected.add(pos, current_word);

        if (pos->type != TokenType::BareWord)
            return false;

        const char * const next_whitespace = find_first_symbols<' ', '\0'>(current_word, s.end());
        const size_t word_length = next_whitespace - current_word;

        if (word_length != pos->size())
            return false;

        if (0 != strncasecmp(pos->begin, current_word, word_length))
            return false;

        ++pos;

        if (!*next_whitespace)
            break;

        current_word = next_whitespace + 1;
    }

    return true;
}


}
