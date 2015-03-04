#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "util.h"
#include "lllparser.h"
#include "bignum.h"
#include "optimize.h"

// Compile-time arithmetic calculations
Node calcArithmetic(Node inp, bool modulo=true) {
    if (inp.type == TOKEN) {
        Node o = tryNumberize(inp);
        if (decimalGt(o.val, tt256, true))
            err("Value too large (exceeds 32 bytes or 2^256)", inp.metadata);
        return o;
    }
	for (unsigned i = 0; i < inp.args.size(); i++) {
        inp.args[i] = calcArithmetic(inp.args[i]);
    }
    // Arithmetic-specific transform
    if (inp.val == "+") inp.val = "add";
    if (inp.val == "*") inp.val = "mul";
    if (inp.val == "-") inp.val = "sub";
    if (inp.val == "/") inp.val = "sdiv";
    if (inp.val == "^") inp.val = "exp";
    if (inp.val == "**") inp.val = "exp";
    if (inp.val == "%") inp.val = "smod";
    // Degenerate cases for add and mul
    if (inp.args.size() == 2) {
        if (inp.val == "add" && inp.args[0].type == TOKEN && 
                inp.args[0].val == "0") {
            Node x = inp.args[1];
            inp = x;
        }
        if (inp.val == "add" && inp.args[1].type == TOKEN && 
                inp.args[1].val == "0") {
            Node x = inp.args[0];
            inp = x;
        }
        if (inp.val == "mul" && inp.args[0].type == TOKEN && 
                inp.args[0].val == "1") {
            Node x = inp.args[1];
            inp = x;
        }
        if (inp.val == "mul" && inp.args[1].type == TOKEN && 
                inp.args[1].val == "1") {
            Node x = inp.args[0];
            inp = x;
        }
    }
    // Arithmetic computation
    if (inp.args.size() == 2 
            && inp.args[0].type == TOKEN 
            && inp.args[1].type == TOKEN) {
      std::string o;
      if (inp.val == "add") {
          o = decimalAdd(inp.args[0].val, inp.args[1].val);
          if (modulo) o = decimalMod(o, tt256);
      }
      else if (inp.val == "sub") {
          if (decimalGt(inp.args[0].val, inp.args[1].val, true))
              o = decimalSub(inp.args[0].val, inp.args[1].val);
      }
      else if (inp.val == "mul") {
          o = decimalMul(inp.args[0].val, inp.args[1].val);
          if (modulo) o = decimalMod(o, tt256);
      }
      else if (inp.val == "div" && inp.args[1].val != "0") {
          o = decimalDiv(inp.args[0].val, inp.args[1].val);
      }
      else if (inp.val == "sdiv" && inp.args[1].val != "0"
            && decimalGt(tt255, inp.args[0].val)
            && decimalGt(tt255, inp.args[1].val)) {
          o = decimalDiv(inp.args[0].val, inp.args[1].val);
      }
      else if (inp.val == "mod" && inp.args[1].val != "0") {
          o = decimalMod(inp.args[0].val, inp.args[1].val);
      }
      else if (inp.val == "smod" && inp.args[1].val != "0"
            && decimalGt(tt255, inp.args[0].val)
            && decimalGt(tt255, inp.args[1].val)) {
          o = decimalMod(inp.args[0].val, inp.args[1].val);
      }    
      else if (inp.val == "exp") {
          if (modulo) o = decimalModExp(inp.args[0].val, inp.args[1].val, tt256);
          else o = decimalExp(inp.args[0].val, inp.args[1].val);
      }
      if (o.length()) return token(o, inp.metadata);
    }
    return inp;
}

// Filter out with statements where the underlying variable
// is only used once
std::pair<Node, int> filterTrivialWiths(Node inp, std::string focus) {
    if (inp.type == TOKEN) {
        return std::pair<Node, int>(inp, (inp.val == focus) ? 1 : 0);
    }
    else if (inp.val != "with") {
        std::vector<Node> o;
        int total = 0;
        for (int i = 0; i < inp.args.size(); i++) {
            std::pair<Node, int> sub = filterTrivialWiths(inp.args[i], focus);
            o.push_back(sub.first);
            total += sub.second;
        }
        return std::pair<Node, int>(asn(inp.val, o, inp.metadata), total);
    }
    else {
        std::string var = inp.args[0].val;
        bool compress = false;
        std::pair<Node, int> sub;
        if (!isDegenerate(inp.args[1])) {
            sub = filterTrivialWiths(inp.args[2], var);
                
        }
    }
}

// Optimize a node (now does arithmetic, may do other things)
Node optimize(Node inp) {
    return calcArithmetic(inp, true);
}

// Is a node degenerate (ie. trivial to calculate) ?
bool isDegenerate(Node n) {
    return optimize(n).type == TOKEN;
}

// Is a node purely arithmetic?
bool isPureArithmetic(Node n) {
    return isNumberLike(calcArithmetic(n));
}