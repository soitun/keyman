/*
  Copyright:    © 2018 SIL International.
  Description:  Tests for kmx integration
  Create Date:  30 Oct 2018
  Authors:      Marc Durdin (MD), Tim Eves (TSE)
*/
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <list>
#include <keyman/keyboardprocessor.h>
#include "state.hpp"

#define   try_status(expr) \
{auto __s = (expr); if (__s != KM_KBP_STATUS_OK) std::exit(100*__LINE__+__s);}

#ifdef assert
#undef assert
#endif
#define assert(expr) {if (!(expr)) std::exit(100*__LINE__); }

std::string utf16_to_utf8(std::u16string utf16_string);

namespace
{

const std::string base = "tests/unit/kmx/";

struct key_event {
  km_kbp_virtual_key vk;
  uint16_t modifier_state;
};

typedef enum {
  KOT_INPUT,
  KOT_OUTPUT
} kmx_option_type;

struct kmx_option {
  kmx_option_type type;
  std::u16string key, value;
};

using kmx_options = std::vector<kmx_option>;

int run_test(const std::string & file);
int load_source(const std::string & file, std::string & keys, std::u16string & expected, std::u16string & context, kmx_options &options);


km_kbp_option_item test_env_opts[] =
{
  KM_KBP_OPTIONS_END
};

// String trim functions from https://stackoverflow.com/a/217605/1836776
// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
  ltrim(s);
  return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
  rtrim(s);
  return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

key_event char_to_event(char ch) {
  assert(ch >= 32 && ch < 128);
  return {
    s_char_to_vkey[(int)ch - 32].vk,
    (uint16_t)(s_char_to_vkey[(int)ch - 32].shifted ?  KM_KBP_MODIFIER_SHIFT : 0)
  };
}

uint16_t const get_modifier(std::string const m) {
  for (int i = 0; s_modifier_names[i].name; i++) {
    if (m == s_modifier_names[i].name) {
      return s_modifier_names[i].modifier;
    }
  }
  return 0;
}

km_kbp_virtual_key const get_vk(std::string const & vk) {
  for (int i = 1; i < 256; i++) {
    if (vk == s_key_names[i]) {
      return i;
    }
  }
  return 0;
}

key_event const vkey_to_event(std::string const & vk_event) {
  // vkey format is MODIFIER MODIFIER K_NAME
  //std::cout << "VK=" << vk_event << std::endl;

  std::stringstream f(vk_event);
  std::string s;
  uint16_t modifier_state = 0;
  km_kbp_virtual_key vk = 0;
  while(std::getline(f, s, ' ')) {
    uint16_t modifier = get_modifier(s);
    if (modifier != 0) {
      modifier_state |= modifier;
    }
    else {
      vk = get_vk(s);
      break;
    }
  }

  // The string should be empty at this point
  assert(!std::getline(f, s, ' '));
  assert(vk != 0);

  return {
    vk,
    modifier_state
  };
}

key_event next_key(std::string &keys) {
  // Parse the next element of the string, chop it off, and return it
  if (keys.length() == 0) return { 0 };
  char ch = keys[0];
  if (ch == '[') {
    if (keys.length() > 1 && keys[1] == '[') {
      keys.erase(0, 2);
      return char_to_event(ch);
    }
    auto n = keys.find(']');
    assert(n != std::string::npos);
    auto vkey = keys.substr(1, n - 1);
    keys.erase(0, n+1);
    return vkey_to_event(vkey);
  }
  else {
    keys.erase(0, 1);
    return char_to_event(ch);
  }
}

void apply_action(km_kbp_state const * state, km_kbp_action_item const & act) {
  switch (act.type)
  {
  case KM_KBP_IT_END:
    assert(false);
    break;
  case KM_KBP_IT_ALERT:
    //std::cout << "beep" << std::endl;
    break;
  case KM_KBP_IT_CHAR:
    //std::cout << "char(" << act.character << ") size=" << cp->size() << std::endl;
    break;
  case KM_KBP_IT_MARKER:
    //std::cout << "deadkey(" << act.marker << ")" << std::endl;
    break;
  case KM_KBP_IT_BACK:
    break;
  case KM_KBP_IT_PERSIST_OPT:
  case KM_KBP_IT_RESET_OPT:
    assert(false); // TODO
    break;
  case KM_KBP_IT_VKEYDOWN:
  case KM_KBP_IT_VKEYUP:
  case KM_KBP_IT_VSHIFTDOWN:
  case KM_KBP_IT_VSHIFTUP:
    assert(false); // NOT SUPPORTED
    break;
  default:
    assert(false); // NOT SUPPORTED
    break;
  }
}

int run_test(const std::string & file) {
  std::string keys = "";
  std::u16string expected = u"", context = u"";
  kmx_options options;
  
  int result = load_source(file, keys, expected, context, options);
  if (result != 0) return result;

  std::cout << "file = " << file << std::endl;
  //std::cout << "keys = " << keys << std::endl;
  //std::cout << "expected = " << utf16_to_utf8(expected) << std::endl;
  //std::cout << "context = " << utf16_to_utf8(context) << std::endl;

  km_kbp_keyboard * test_kb = nullptr;
  km_kbp_state * test_state = nullptr;
    
  try_status(km_kbp_keyboard_load(std::filesystem::path(base + file + ".kmx").c_str(), &test_kb));

  // Setup state, environment

  try_status(km_kbp_state_create(test_kb, test_env_opts, &test_state));

  // Setup keyboard options

  if (options.size() > 0) {
    km_kbp_option_item *keyboard_opts = new km_kbp_option_item[options.size() + 1];

    int i = 0;
    for (auto it = options.begin(); it != options.end(); it++) {
      if (it->type != KOT_INPUT) continue;

      std::cout << "input option-key: " << utf16_to_utf8(it->key) << std::endl;

      keyboard_opts[i].key = new km_kbp_cp[it->key.length() + 1];
      it->key.copy((char16_t * const)keyboard_opts[i].key, it->key.length());
      keyboard_opts[i].key[it->key.length()] = 0;

      keyboard_opts[i].value = new km_kbp_cp[it->value.length() + 1];
      it->value.copy(keyboard_opts[i].value, it->value.length());
      keyboard_opts[i].value[it->value.length()] = 0;

      keyboard_opts[i].scope = KM_KBP_OPT_KEYBOARD;
      i++;
    }

    keyboard_opts[i] = KM_KBP_OPTIONS_END;

    try_status(km_kbp_options_update(test_state, keyboard_opts));

    delete keyboard_opts;
  }

  // Setup context
  km_kbp_context_item *citems = nullptr;
  try_status(km_kbp_context_items_from_utf16(context.c_str(), &citems));
  try_status(km_kbp_context_set(km_kbp_state_context(test_state), citems));
  km_kbp_context_items_dispose(citems);

  // Run through key events, applying output for each event
  for (auto p = next_key(keys); p.vk != 0; p = next_key(keys)) {
    try_status(km_kbp_process_event(test_state, p.vk, p.modifier_state));
    for (auto act = km_kbp_state_action_items(test_state, nullptr); act->type != KM_KBP_IT_END; act++) {
      apply_action(test_state, *act);
    }
  }

  // Compare final output { TODO: don't use the inbuilt context, instead use the actions, starting with context }
  size_t n = 0;
  try_status(km_kbp_context_get(km_kbp_state_context(test_state), &citems));
  try_status(km_kbp_context_items_to_utf16(citems, nullptr, &n));
  km_kbp_cp *buf = new km_kbp_cp[n];
  try_status(km_kbp_context_items_to_utf16(citems, buf, &n));
  km_kbp_context_items_dispose(citems);

  // Test resultant options
  // TODO: test also KM_KBP_IT_PERSIST_OPT and KM_KBP_IT_RESET_OPT actions
  
  for (auto it = options.begin(); it != options.end(); it++) {
    if (it->type != KOT_OUTPUT) continue;
    std::cout << "output option-key: " << utf16_to_utf8(it->key) << " expected: " << utf16_to_utf8(it->value);
    km_kbp_cp const *value;
    try_status(km_kbp_options_lookup(test_state, KM_KBP_OPT_KEYBOARD, it->key.c_str(), &value));
    std::cout << " actual: " << utf16_to_utf8(value) << std::endl;
    if (it->value.compare(value) != 0) return __LINE__;
    km_kbp_cp_dispose(value);
  }

  // Destroy them
  km_kbp_state_dispose(test_state);
  km_kbp_keyboard_dispose(test_kb);

  return (buf == expected) ? 0 : __LINE__;
}

std::u16string parse_source_string(std::string const & s) {
  std::u16string t;
  for (auto p = s.begin(); p != s.end(); p++) {
    if (*p == '\\') {
      p++;
      km_kbp_usv v;
      assert(p != s.end());
      if (*p == 'u') {
        // Unicode value
        p++;
        size_t n;
        std::string s1 = s.substr(p - s.begin(), 6);
        v = std::stoul(s1, &n, 16);
        assert(v >= 0x20 && v <= 0x10FFFF);
        p += n-1;
        if (v < 0x10000) {
          t += km_kbp_cp(v);
        }
        else {
          t += km_kbp_cp(Uni_UTF32ToSurrogate1(v)) + km_kbp_cp(Uni_UTF32ToSurrogate2(v));
        }
      }
      else if (*p == 'd') {
        // Deadkey
        // TODO, not yet supported
        assert(false);
      }
    }
    else {
      t += *p;
    }
  }
  return t;
}

bool parse_option_string(std::string line, kmx_options &options, kmx_option_type type) {
  auto x = line.find('=');
  if (x == std::string::npos) return false;
  
  kmx_option o;
  
  o.type = type;
  o.key = parse_source_string(line.substr(0, x));
  o.value = parse_source_string(line.substr(x + 1));

  options.emplace_back(o);
  return true;
}

bool is_token(const std::string token, std::string &line) {
  if (line.compare(0, token.length(), token) == 0) {
    line = line.substr(token.length());
    trim(line);
    return true;
  }
  return false;
}

int load_source(const std::string & file, std::string & keys, std::u16string & expected, std::u16string & context, kmx_options &options) {
  const std::string s_keys = "c keys: ",
    s_expected = "c expected: ",
    s_context = "c context: ",
    s_option = "c option: ",
    s_option_expected = "c expected option: ";

  // Parse out the header statements in file.kmn that tell us (a) environment, (b) key sequence, (c) start context, (d) expected result
  std::ifstream kmn(base + file + ".kmn");
  if (!kmn.good()) {
    kmn.open(file + ".kmn");
  }
  if (!kmn.good()) {
    return __LINE__;
  }
  std::string line;
  while (std::getline(kmn, line)) {
    trim(line);

    if (!line.length()) continue;
    if (line.compare(0, s_keys.length(), s_keys) == 0) {
      keys = line.substr(s_keys.length());
      trim(keys);
    }
    else if(is_token(s_expected, line)) {
      expected = parse_source_string(line);
    }
    else if (is_token(s_context, line)) {
      context = parse_source_string(line);
    }
    else if (is_token(s_option, line)) {
      if (!parse_option_string(line, options, KOT_INPUT)) return __LINE__;
    }
    else if (is_token(s_option_expected, line)) {
      if (!parse_option_string(line, options, KOT_OUTPUT)) return __LINE__;
    }
  }

  if (keys == "") {
    // We must at least have a key sequence to run the test
    return __LINE__;
  }

  return 0;
}

} // namespace

int main(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    int result;
    std::cout << argv[i] << std::endl;
    if ((result = run_test(argv[i])) != 0) return result;
  }
  return 0;
}


