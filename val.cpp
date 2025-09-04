// Name: Bassil Saleh
// Class: CS 280, section 007
// Value class implementation

#include "val.h"
#include <cmath>    // FP_ZERO and fpclassify()
#include <limits>   // epsilon() for double values

bool FPEqual(double x, double y) {
    // Compare two floating point values
//    return (abs(x-y) <= std::numeric_limits<double>::epsilon());
    return (abs(x-y) <= 0.0001);
}

Value Value::operator+(const Value& op) const {
    Value result;   // error type by default
    if (this->IsInt()) {
        if (op.IsInt()) {
            // int + int = int
            int x = this->GetInt();
            int y = op.GetInt();
            result.SetType(VINT);
            result.SetInt(x + y);
        }
        else if (op.IsReal()) {
            // int + real = real
            double x = static_cast<double>(this->GetInt());
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x + y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    else if (this->IsReal()) {
        if (op.IsInt()) {
            // real + int = real
            double x = this->GetReal();
            double y = static_cast<double>(op.GetInt());
            result.SetType(VREAL);
            result.SetReal(x + y);
        }
        else if (op.IsReal()) {
            // real + real = real
            double x = this->GetReal();
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x + y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    // If caller's ValType is not int or real, then
    // it is either boolean, string, or error.
    return result;
}

Value Value::operator-(const Value& op) const {
    Value result;   // error type by default
    if (this->IsInt()) {
        if (op.IsInt()) {
            // int - int = int
            int x = this->GetInt();
            int y = op.GetInt();
            result.SetType(VINT);
            result.SetInt(x-y);
        }
        else if (op.IsReal()) {
            // int - real = real
            double x = static_cast<double>(this->GetInt());
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x-y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    else if (this->IsReal()) {
        if (op.IsInt()) {
            // real - int = real
            double x = this->GetReal();
            double y = static_cast<double>(op.GetInt());
            result.SetType(VREAL);
            result.SetReal(x-y);
        }
        else if (op.IsReal()) {
            // real - real = real
            double x = this->GetReal();
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x-y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    // If caller's ValType is not int or real, then
    // it is either boolean, string, or error.
    return result;
}

Value Value::operator*(const Value& op) const {
    Value result;
    if (this->IsInt()) {
        if (op.IsInt()) {
            // int * int = int
            int x = this->GetInt();
            int y = op.GetInt();
            result.SetType(VINT);
            result.SetInt(x*y);
        }
        else if (op.IsReal()) {
            // int * real = real
            double x = static_cast<double>(this->GetInt());
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x*y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    else if (this->IsReal()) {
        if (op.IsInt()) {
            // real * int = real
            double x = this->GetReal();
            double y = static_cast<double>(op.GetInt());
            result.SetType(VREAL);
            result.SetReal(x*y);
        }
        else if (op.IsReal()) {
            // real * real = real
            double x = this->GetReal();
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x*y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    return result;
}

Value Value::operator/(const Value& op) const {
    Value result;   // error type by default
    // This operator is not defined for
    // string, boolean, or error types
    if (this->IsErr() || this->IsBool() || this->IsString() ||
        op.IsErr() || op.IsBool() || op.IsString()) {
            return result;
        }
    // If op is zero, return
    // result immediately to
    // avoid division by zero.
    if (op.IsInt()) {
        if (op.GetInt() == 0) {
            return result;
        }
    }
    else if (op.IsReal()) {
        if (FP_ZERO == fpclassify(op.GetReal())) {
            return result;
        }
    }
    // int / int or int / real
    if (this->IsInt()) {
        int x = this->GetInt();
        if (op.IsInt()) {
            // int / int = int
            int y = op.GetInt();
            result.SetType(VINT);
            result.SetInt(x / y);
        }
        else if (op.IsReal()) {
            // int / real = real
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x / y);
        }
    }
    // real / real or real / int
    else if (this->IsReal()) {
        double x = this->GetReal();
        if (op.IsInt()) {
            // real / int = real
            int y = op.GetInt();
            result.SetType(VREAL);
            result.SetReal(x / y);
        }
        else if (op.IsReal()) {
            // real / real = real
            double y = op.GetReal();
            result.SetType(VREAL);
            result.SetReal(x / y);
        }
    }
    return result;
}

Value Value::operator%(const Value& oper) const {
    // Modulus only works with two int types
    Value result;
    if (this->IsInt() && oper.IsInt()) {
        // Avoid vision by zero
        if (oper.GetInt() == 0) {
            return result;  // is error type by default
        }
        int x = this->GetInt();
        int y = oper.GetInt();
        result.SetType(VINT);
        result.SetInt(x % y);
    }
    return result;
}

Value Value::div(const Value& oper) const {
    // unsigned integer division
    Value result;   // error type by default
    if (this->IsInt()) {
        if (oper.IsInt()) {
            // If oper is zero, return
            // result immediately to
            // avoid division by zero.
            if (oper.GetInt() == 0) { return result; }
            // int div int = int
            int x = this->GetInt();
            int y = oper.GetInt();
            result.SetType(VINT);
            result.SetInt(x/y);
        }
        else if (oper.IsReal()) {
            // Avoid division by zero
            if (FP_ZERO == fpclassify(oper.GetReal())) { return result; }
            // int div real = int
            int x = this->GetInt();
            int y = static_cast<int>(oper.GetReal());
            result.SetType(VINT);
            result.SetInt(x/y);
        }
        else {
            // oper is not int or real
            return result;
        }
    }
    else if (this->IsReal()) {
        if (oper.IsInt()) {
            // Avoid division by zero
            if (oper.GetInt() == 0) { return result; }
            // real div int = int
            int x = static_cast<int>(this->GetReal());
            int y = oper.GetInt();
            result.SetType(VINT);
            result.SetInt(x/y);
        }
        else {
            // oper is not int
            return result;
        }
    }
    // If caller's ValType is not int or real,
    // then it is either boolean, string, or error.
    return result;
}

Value Value::operator==(const Value& op) const {
    Value result;   // error type by default
    // Should ERROR == ERROR equal ERROR, or true?
    if (!this->IsErr() && !op.IsErr()) {
        if (this->IsInt()) {
            if (op.IsString() || op.IsBool()) {
                // int == str = ERROR
                // int == bool = ERROR
                return result;
            }
            else if (!op.IsInt()) {
                // int == real becomes real == real
                double x = static_cast<double>(this->GetInt());
                double y = op.GetReal();
                result.SetType(VBOOL);
                result.SetBool(FPEqual(x,y));
                return result;
            }
            // int == int
            int x = this->GetInt();
            int y = op.GetInt();
            result.SetType(VBOOL);
            result.SetBool(x == y);
        }
        else if (this->IsReal()) {
            if (op.IsString() || op.IsBool()) {
                // real == str = ERROR
                // real == bool = ERROR
                return result;
            }
            else if (!op.IsReal()) {
                // real == int becomes real == real
                double x = this->GetReal();
                double y = static_cast<double>(op.GetInt());
                result.SetType(VBOOL);
                result.SetBool(FPEqual(x,y));
                return result;
            }
            // real == real
            double x = this->GetReal();
            double y = op.GetReal();
            result.SetType(VBOOL);
            // Comparing doubles using == usually doesn't work in C++
            result.SetBool(FPEqual(x, y));
        }
        else if (this->IsString()) {
            if (op.IsInt() || op.IsReal() || op.IsBool()) {
                // str == int = ERROR
                // str == real = ERROR
                // str == bool = ERROR
                return result;
            }
            // str == str is fine
            string x = this->GetString();
            string y = op.GetString();
            result.SetType(VBOOL);
            result.SetBool(x == y);
        }
        else if (this->IsBool()) {
            // bool == int = ERROR
            // bool == real = ERROR
            // bool == string = ERROR
            if (op.IsInt() || op.IsReal() || op.IsString()) {
                return result;
            }
            bool x = this->GetBool();
            bool y = op.GetBool();
            result.SetType(VBOOL);
            result.SetBool(x == y);
        }
    }
    return result;
}

Value Value::operator>(const Value& op) const {
    Value result;   // error type by default
    if (this->IsInt()) {
        if (op.IsInt()) {
            // int > int
            int x = this->GetInt();
            int y = op.GetInt();
            result.SetType(VBOOL);
            result.SetBool(x>y);
        }
        else if (op.IsReal()) {
            // int > real becomes real > real
            double x = static_cast<double>(this->GetInt());
            double y = op.GetReal();
            result.SetType(VBOOL);
            result.SetBool(x>y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    else if (this->IsReal()) {
        if (op.IsInt()) {
            // real > int becomes real > real
            double x = this->GetReal();
            double y = static_cast<double>(op.GetInt());
            result.SetType(VBOOL);
            result.SetBool(x>y);
        }
        else if (op.IsReal()) {
            // real > real
            double x = this->GetReal();
            double y = op.GetReal();
            result.SetType(VBOOL);
            result.SetBool(x>y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    // If caller's ValType is not int or real,
    // then it is either boolean, string or error
    return result;
}

Value Value::operator<(const Value& op) const {
    Value result;   // error type by default
    if (this->IsInt()) {
        if (op.IsInt()) {
            // int < int
            int x = this->GetInt();
            int y = op.GetInt();
            result.SetType(VBOOL);
            result.SetBool(x<y);
        }
        else if (op.IsReal()) {
            // int < real becomes real < real
            double x = static_cast<double>(this->GetInt());
            double y = op.GetReal();
            result.SetType(VBOOL);
            result.SetBool(x<y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    else if (this->IsReal()) {
        if (op.IsInt()) {
            // real < int becomes real < real
            double x = this->GetReal();
            double y = static_cast<double>(op.GetInt());
            result.SetType(VBOOL);
            result.SetBool(x<y);
        }
        else if (op.IsReal()) {
            // real < real
            double x = this->GetReal();
            double y = op.GetReal();
            result.SetType(VBOOL);
            result.SetBool(x<y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    // If caller's ValType is not int or real,
    // then it is either boolean, string or error
    return result;
}

Value Value::idiv(const Value& op) const {
    // signed integer division
    Value result;   // error type by default
    if (this->IsInt()) {
        if (op.IsInt()) {
            // int div int = int
            int x = this->GetInt();
            int y = op.GetInt();
            result.SetType(VINT);
            result.SetInt(x/y);
        }
        else if (op.IsReal()) {
            // int div real = int
            int x = this->GetInt();
            int y = static_cast<int>(op.GetReal());
            result.SetType(VINT);
            result.SetInt(x/y);
        }
        else {
            // op is not int or real
            return result;
        }
    }
    else if (this->IsReal()) {
        if (op.IsInt()) {
            // real div int = int
            int x = static_cast<int>(this->GetReal());
            int y = op.GetInt();
            result.SetType(VINT);
            result.SetInt(x/y);
        }
        else {
            // op is not int
            return result;
        }
    }
    // If caller's ValType is not int or real,
    // then it is either boolean, string, or error.
    return result;
}

Value Value::operator&&(const Value& oper) const {
    Value result;
    // && only works with two boolean types
    if (this->IsBool() && oper.IsBool()) {
        bool x = this->GetBool();
        bool y = oper.GetBool();
        result.SetType(VBOOL);
        result.SetBool(x && y);
    }
    return result;
}

Value Value::operator||(const Value& oper) const {
    Value result;   // error type by default
    // bool || bool
    if (this->IsBool() && oper.IsBool()) {
        bool x = this->GetBool();
        bool y = oper.GetBool();
        result.SetType(VBOOL);
        result.SetBool(x || y);
    }
    return result;
}

Value Value::operator!() const {
    Value result;   // error type by default
    if (this->IsBool()) {
        bool x = this->GetBool();
        result.SetType(VBOOL);
        result.SetBool(!x);
    }
    return result;
}
